#include <arpa/inet.h>
#include <asm/socket.h>
#include <errno.h>
#include <inttypes.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>

#include "list.h"
#include "log.h"
#include "message.h"
#include "player.h"

/**
 * get and bind a socket or exit on failure
 */
int make_socket(char *host, uint16_t port) {
	int sock;
	struct sockaddr_in name;

	// get a socket
	sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	// forcefully attaching socket to the port
	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt,
	               sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons(port);
	inet_pton(AF_INET, host, &name.sin_addr.s_addr);
	if (bind(sock, (struct sockaddr *)&name, sizeof(name)) < 0) {
		perror("bind");
		exit(EXIT_FAILURE);
	}

	return sock;
}

static void tell_party_that_the_game_started(TetrisParty *party) {
	int i;
	Player *player;
	List *players = ttetris_party_get_players(party);

	StringArray *party_members =
	    string_array_create(players->length, PLAYER_NAME_MAX_CHARS);
	for (i = 0; i < players->length; i++)
		string_array_set_item(party_members, i,
		                      ((Player *)list_get(players, i))->name);
	Blob *party_members_blob = string_array_serialize(party_members);

	for (i = 0; i < players->length; i++) {
		player = (Player *)list_get(players, i);
		if (player->fd)
			message_blob(player->fd, party_members_blob, 0,
			             MSG_TYPE_GAME_STARTED);
	}
}

/**
 * Returns -1 if EOF is received or 0 otherwise.
 */
int read_from_client(int filedes) {
	char buffer[MAXMSG];
	Blob *blob;

	// remember that more than one TCP packet may be read by this command
	int nbytes = read(filedes, buffer, MAXMSG);

	// exit early if there was an error
	if (nbytes < 0) {
		perror("read");
		exit(EXIT_FAILURE);
	}

	// exit early if we reached the end-of-file
	if (nbytes == 0)
		return -1;

	// data was successfully read into the buffer
	fprintf(stderr, "read_from_client: received %d bytes from client\n",
	        nbytes);

	char *end = buffer + nbytes;
	char *cursor = buffer;

	Player *opponent;
	Player *player = get_player_from_fd(filedes);
	if (player == 0) {
		printf("read_from_client: player is null for socket file "
		       "descriptor.\n");
	}
	char name[16];

	while (cursor < end) {
		MessageHeader *header = (MessageHeader *)buffer;
		fprintf(stderr,
		        "read_from_client: magic=0x%x id=%d n_bytes=%d "
		        "msg_type=%s\n",
		        header->magic_number, header->request_id,
		        header->content_length,
		        message_type_to_str(header->message_type));

		// increment the cursor to the start of the message body
		cursor += sizeof(MessageHeader);

		switch (header->message_type) {
		case MSG_TYPE_START_GAME:
			if (player->party == 0)
				break;
			ttetris_party_start(player->party);
			tell_party_that_the_game_started(player->party);
			break;
		case MSG_TYPE_REGISTER:
			sscanf(cursor, "%15s", name);
			player = player_create(filedes, name);
			player->render = send_player;
			message_nbytes(filedes, NULL, 0, header->request_id,
			               MSG_TYPE_REGISTER_SUCCESS);
			break;
		case MSG_TYPE_ROTATE:
			rotate_block(cursor[0], player->contents);
			break;
		case MSG_TYPE_TRANSLATE:
			translate_block(cursor[0], player->contents);
			break;
		case MSG_TYPE_LOWER:
			lower_block(0, player->contents);
			break;
		case MSG_TYPE_DROP:
			hard_drop(player->contents);
			break;
		case MSG_TYPE_SWAP_HOLD:
			swap_hold_block(player->contents);
			break;
		case MSG_TYPE_OPPONENT:
			blob = malloc(sizeof(blob));
			blob->bytes = cursor;
			blob->length = header->content_length;
			StringArray *opponent_names =
			    string_array_deserialize(blob);

			TetrisParty *party = ttetris_party_create();
			ttetris_party_player_add(party, player);

			for (int i = 0; i < opponent_names->length; i++) {

				opponent = player_get_by_name(
				    string_array_get_item(opponent_names, i));

				if (!opponent) {
					fprintf(logging_fp,
					        "read_from_client: Could "
					        "not find opponent\n");
					break;
				}
				fprintf(stderr,
				        "read_from_client: Adding opponent "
				        "number %d to party: %s\n",
				        i, opponent->name);
				ttetris_party_player_add(party, opponent);
			}

			break;
		case MSG_TYPE_LIST:
			send_online_users(filedes, header->request_id);
			break;
		default:
			fprintf(stderr,
			        "read_from_client:_received unrecognized "
			        "message with message type 0x%x",
			        header->message_type);
		}

		// increment the cursor past the message body end
		cursor += header->content_length;
	}

	if (player) {
		// if the player has a party, send the board to all players
		if (player->party) {
			List *party_members =
			    ttetris_party_get_players(player->party);
			for (int i = 0; i < party_members->length; i++)
				send_player(
				    ((Player *)list_get(party_members, i))->fd,
				    player);
		}
		// otherwise, just send the board to the player
		else {
			send_player(player->fd, player);
		}
	}

	return 0;
}

void usage() {
	fprintf(stderr, "Usage: ./server [-h] [-a ADDRESS] [-p PORT]\n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	char host[128] = "127.0.0.1";
	char port[6] = "5555";

	// set the logger file pointer to stderr
	logging_set_fp(stderr);

	// Parse command line flags. The optstring passed to getopt has a
	// preceding colon to tell getopt that missing flag values should be
	// treated differently than unknown flags. The proceding colons indicate
	// that flags must have a value.
	int opt;
	while ((opt = getopt(argc, argv, ":ha:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 'a':
			strncpy(host, optarg, 127);
			printf("address: %s\n", optarg);
			break;
		case 'p':
			strncpy(port, optarg, 5);
			printf("port: %s\n", optarg);
			break;
		case ':':
			printf("option -%c needs a value\n", optopt);
			break;
		case '?':
			printf("unknown option: %c\n", optopt);
			break;
		}
	}

	// convert the string port to a number port
	uintmax_t numeric_port = strtoumax(port, NULL, 10);
	if (numeric_port == UINTMAX_MAX && errno == ERANGE) {
		fprintf(stderr, "Provided port is invalid\n");
		usage();
	}

	int sock;
	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size;

	/* Create the socket and set it up to accept connections. */
	sock = make_socket(host, numeric_port);
	if (listen(sock, 1) < 0) {
		perror("listen");
		exit(EXIT_FAILURE);
	}

	/* Initialize the set of active sockets. */
	FD_ZERO(&active_fd_set);
	FD_SET(sock, &active_fd_set);

	/* Initialize the player list */
	player_init();

	while (1) {
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
			perror("select");
			exit(EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i) {
			// exit early if the file descriptor i is not in the set
			if (!FD_ISSET(i, &read_fd_set))
				continue;

			// for new connections:
			// - accept the connection
			// - create a file descriptor for the connection
			// - add the file descriptor to the file descriptor set
			if (i == sock) {
				size = sizeof(clientname);
				int new =
				    accept(sock, (struct sockaddr *)&clientname,
				           (socklen_t *)&size);
				if (new < 0) {
					perror("accept");
					exit(EXIT_FAILURE);
				}
				fprintf(stderr,
				        "main: new connection from host %s, "
				        "port %hu.\n",
				        inet_ntoa(clientname.sin_addr),
				        ntohs(clientname.sin_port));

				FD_SET(new, &active_fd_set);
			}
			// handle data on sockets already in the file descriptor
			// set
			else if (read_from_client(i) < 0) {
				fprintf(stderr, "main: received EOF\n");
				close(i);
				FD_CLR(i, &active_fd_set);
			}
		}
	}
}

// vi:noet:noai:sw=0:sts=0:ts=8
