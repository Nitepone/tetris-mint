#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "player.h"

// message must hold 24 * 10 integers
#define MAXMSG 1024

#define MSG_TYPE_REGISTER 'U'
#define MSG_TYPE_OPPONENT 'O'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'
#define MSG_TYPE_DROP 'D'

void send_board (struct st_player * player);

/**
 * get and bind a socket or exit on failure
 */
int
make_socket (char * host, uint16_t port)
{
	int sock;
	struct sockaddr_in name;

	// get a socket
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror ("socket");
		exit (EXIT_FAILURE);
	}

	// forcefully attaching socket to the port
	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	inet_pton(AF_INET, host, &name.sin_addr.s_addr);
	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror ("bind");
		exit (EXIT_FAILURE);
	}

	return sock;
}

/**
 * Returns -1 if EOF is received or 0 otherwise.
 */
int
read_from_client (int filedes)
{
	fprintf(stderr, "read_from_client()\n");

	char buffer[MAXMSG];

	// remember that more than one TCP packet may be read by this command
	int nbytes = read (filedes, buffer, MAXMSG);

	// exit early if there was an error
	if (nbytes < 0) {
		perror ("read");
		exit (EXIT_FAILURE);
	}

	// exit early if we reached the end-of-file
	if (nbytes == 0)
		return -1;

	// data was successfully read into the buffer
	fprintf (stderr, "read_from_client: received %d bytes from client\n", nbytes);


	char * end = buffer + nbytes;
	char * cursor = buffer;

	Player * player = get_player_from_fd(filedes);
	char name[16];

	while( cursor < end )
	{
		// the first two bytes of the message should be used to indicate the
		// remaining number of bytes in the message
		uint16_t message_size = (buffer[0] << 8) + buffer[1];
		fprintf (stderr, "read_from_client: message size is %d\n", message_size);

		// increment the cursor to the start of the message body
		cursor += 2;
		fprintf( stderr, "message body: %s\n", cursor);

		switch(cursor[0]){
		case MSG_TYPE_REGISTER:
			sscanf( cursor + 1, "%15s", name);
			player = player_create(filedes, name);
			player->render = send_board;
			break;
		case MSG_TYPE_ROTATE:
			rotate_block(cursor[1], player->contents);
			break;
		case MSG_TYPE_TRANSLATE:
			translate_block(cursor[1], player->contents);
			break;
		case MSG_TYPE_LOWER:
			lower_block(0, player->contents);
			break;
		case MSG_TYPE_DROP:
			hard_drop (player->contents);
			break;
		case MSG_TYPE_OPPONENT:
			fprintf(stderr, "Opponent: %s\n", cursor + 1);
			break;
		default:
			fprintf(stderr, "read_from_client:_received unrecognized "
			       "message with starting byte 0x%x", cursor[0]);
		}

		// increment the cursor past the message body end
		cursor += message_size;
	}
	send_board(player);

	return 0;
}

void
send_client_nbytes (int fd, char * message, int n)
{
  int bytes_written = write (fd, message, n);
  fprintf(stderr, "Sent %d bytes to client\n", bytes_written);

  if (bytes_written < 0) {
    perror ("write");
    exit (EXIT_FAILURE);
  }
}

void
message_client (int fd, char * message)
{
  int nbytes = write (fd, message, strlen (message) + 1);
  if (nbytes < 0) {
    perror ("write");
    exit (EXIT_FAILURE);
  }
}

void
send_board (struct st_player * player)
{
	if (player == 0) {
		fprintf(stderr, "Error: No player found for socket\n");
		return;
	}

	if (player->view == 0) {
		fprintf(stderr, "Error: No view exists for player.\n");
		return;
	}

	if (player->view->board == 0) {
		fprintf(stderr, "Error: No board exists for player.\n");
		return;
	}

	char message[MAXMSG] = "BOARD";
	generate_game_view_data(&player->view, player->contents);
	memcpy(message + 5, player->view->board, 960);
	send_client_nbytes(player->fd, message, 965);
}

void
usage()
{
  fprintf(stderr, "Usage: ./server ADDRESS PORT\n");
  exit(EXIT_FAILURE);
}

int
main(int argc, char * argv[])
{
	char host[128] = "127.0.0.1";
	char port[6] = "5555";

	// Parse command line flags. The optstring passed to getopt has a preceding
	// colon to tell getopt that missing flag values should be treated
	// differently than unknown flags. The proceding colons indicate that flags
	// must have a value.
	int opt;
	while((opt = getopt(argc, argv, ":h:p:")) != -1)
	{
		switch(opt)
		{
		case 'h':
			strncpy(host, optarg, 127);
			printf("host: %s\n", optarg);
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
	sock = make_socket (host, numeric_port);
	if (listen (sock, 1) < 0)
		{
			perror ("listen");
			exit (EXIT_FAILURE);
		}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);

	/* Initialize the player list */
	player_init();

	while (1)
	{
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
		{
			perror ("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
		{
			// exit early if the file descriptor i is not in the set
			if (!FD_ISSET (i, &read_fd_set))
				continue;

			// for new connections:
			// - accept the connection
			// - create a file descriptor for the connection
			// - add the file descriptor to the file descriptor set
			if (i == sock)
			{
				size = sizeof (clientname);
				int new = accept (sock,
					(struct sockaddr *) &clientname,
					(socklen_t *) &size);
				if (new < 0)
				{
					perror ("accept");
					exit (EXIT_FAILURE);
				}
				fprintf (stderr,
					 "Server: new connection from host %s, port %hd.\n",
					 inet_ntoa (clientname.sin_addr),
					 ntohs (clientname.sin_port));

				FD_SET (new, &active_fd_set);
			}
			// handle data on sockets already in the file descriptor set
			else if (read_from_client (i) < 0) {
				fprintf(stderr, "Received EOF\n");
				close (i);
				FD_CLR (i, &active_fd_set);
			}
		}
	}
}

// vi:noet:noai:sw=0:sts=0:ts=8
