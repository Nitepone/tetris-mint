
#include <inttypes.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

#include "os_compat.h"
#ifdef THIS_IS_WINDOWS
#include <winsock.h>
#else
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "client_conn.h"
#include "event.h"
#include "log.h"
#include "message.h"
#include "render.h"
#include "tetris_game.h"

// for backward compatibility
#define h_addr h_addr_list[0]

/**
 * Borrowed from GNU Socket Tutorial
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
static int init_sockaddr(struct sockaddr_in *name, const char *hostname,
                         uint16_t port) {
	struct hostent *hostinfo;

	name->sin_family = AF_INET;
	name->sin_port = htons(port);
	hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) {
		fprintf(logging_fp, "Unknown host %s.\n", hostname);
		return EXIT_FAILURE;
	}
	name->sin_addr = *(struct in_addr *)hostinfo->h_addr;

	return EXIT_SUCCESS;
}

void tetris_send_message(NetClient *net_client, char *body,
                         msg_type_t message_type) {
	uint16_t len = strlen(body) + 1;
	ttetris_net_request(net_client, body, len, message_type);
}

/**
 * get socket or exit if an error occurs
 */
int get_socket() {
#ifdef THIS_IS_WINDOWS
	WSADATA wsaData;
	int startup_result;
	// perform the required initialization for winsock
	if ((startup_result = WSAStartup(MAKEWORD(2, 2), &wsaData)) != 0) {
		fprintf(logging_fp, "WSAStartup failed with error: %d\n",
		        startup_result);
		return EXIT_FAILURE;
	}
#endif

	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock >= 0)
		return sock;

	perror("socket (client)");
	return EXIT_FAILURE;
}

static void tetris_translate(void *net_client, int x) {
	char xdir = x > 0 ? 1 : 0;
	char message[2];
	sprintf(message, "%c", xdir);
	tetris_send_message((NetClient *)net_client, message,
	                    MSG_TYPE_TRANSLATE);
}

static void tetris_lower(void *net_client) {
	ttetris_net_request((NetClient *)net_client, NULL, 0, MSG_TYPE_LOWER);
}

static void tetris_rotate(void *net_client, int theta) {
	char dir = theta > 0 ? 1 : 0;
	char message[2];
	sprintf(message, "%c", dir);
	tetris_send_message((NetClient *)net_client, message, MSG_TYPE_ROTATE);
}

static void tetris_drop(void *net_client) {
	ttetris_net_request((NetClient *)net_client, NULL, 0, MSG_TYPE_DROP);
}

static void tetris_swap_hold(void *net_client) {
	ttetris_net_request((NetClient *)net_client, NULL, 0,
	                    MSG_TYPE_SWAP_HOLD);
}

StringArray *tetris_list(NetClient *net_client) {
	NetRequest *request =
	    ttetris_net_request(net_client, NULL, 0, MSG_TYPE_LIST);
	ttetris_net_request_block_for_response(request);

	MessageHeader *header = (MessageHeader *)request->cursor;

	// deserialize names
	Blob *body = malloc(sizeof(Blob));
	body->bytes = request->cursor;
	body->length = header->content_length;
	return string_array_deserialize(body);
}

/**
 * establish a connection to the server
 */
int tetris_connect(NetClient *net_client, char *host, int port) {
	struct sockaddr_in servername;

	/* Create the socket. */
	SOCKET sock_fd = get_socket();

	/* Connect to the server. */
	init_sockaddr(&servername, host, port);
	if (0 > connect(sock_fd, (struct sockaddr *)&servername,
	                sizeof(servername))) {
		return EXIT_FAILURE;
	}

	net_client->fd = sock_fd;

	return EXIT_SUCCESS;
}

/**
 * register
 */
NetRequest *tetris_register(NetClient *net_client, char *username) {
	char message[16];
	strncpy(message, username, 16);
	return ttetris_net_request(net_client, message, 16, MSG_TYPE_REGISTER);
}

/**
 * select opponent by username
 */
void tetris_opponent(NetClient *net_client, StringArray *usernames) {
	Blob *message = string_array_serialize(usernames);
	ttetris_net_request(net_client, message->bytes, message->length,
	                    MSG_TYPE_OPPONENT);
	free(message->bytes);
	free(message);
}

void tetris_tell_server_to_start(NetClient *net_client) {
	message_nbytes(net_client->fd, NULL, 0, 0, MSG_TYPE_START_GAME);
}

void tetris_disconnect(NetClient *net_client) {
	close(net_client->fd);
	net_client->is_listen_thread_started = 0;
}

static int read_game_view_data(char *buffer, struct game_view_data *view) {
	// get the player associated with the board
	char *board_name = buffer;
	unsigned int name_length = strnlen(board_name, PLAYER_NAME_MAX_CHARS);
	// move the pointer past the name string
	buffer += name_length + 1;
	// copy the game view data
	memcpy(view, buffer, sizeof(struct game_view_data));
	render_game_view_data(board_name, view);

	return EXIT_SUCCESS;
}

void ttetris_net_request_complete(NetRequest *request) {
	fprintf(logging_fp, "ttetris_net_request_complete\n");
	ttetris_event_mark_complete(request->response_event);
};

int read_from_server(NetClient *net_client) {
	Blob *blob;
	char buffer[MAXMSG];

	fprintf(logging_fp, "read_from_server: called\n");

	// remember that more than one TCP packet may be read by this command
	int nbytes = recv(net_client->fd, buffer, MAXMSG, 0);

	if (nbytes == 0) {
		// exit early if we reached the end-of-file
		fprintf(logging_fp, "read_from_server: received EOF\n");
		return -1;
	} else if (nbytes < 0) {
		// exit early if there was an error
		char errmsg[256];
		last_error_message_to_buffer(errmsg, 256);
		fprintf(logging_fp, "read_from_server: %s\n", errmsg);
		return EXIT_FAILURE;
	} else {
		fprintf(logging_fp, "read_from_server read %d bytes\n", nbytes);
	}

	// initialize pointers for moving through data
	char *end = buffer + nbytes;
	char *cursor = buffer;

	while (cursor < end) {
		MessageHeader *header = (MessageHeader *)cursor;

		// Check the magic number, used a mechanism to detect errors.
		// For now, we won't fail and exit, but we could consider doing
		// that in the future.
		if (header->magic_number != MSG_MAGIC_NUMBER) {
			fprintf(logging_fp,
			        "read_from_server: incorrect magic number\n");
			return EXIT_SUCCESS;
		}

		fprintf(logging_fp,
		        "read_from_server: message type=%s request_id=%d "
		        "content_length=%d\n",
		        message_type_to_str(header->message_type),
		        header->request_id, header->content_length);

		// increment the cursor to the start of the message body
		cursor += sizeof(MessageHeader);

		//
		// This switch statement is essentially for actions that should
		// be taken for incoming messages. For synchronous "requests",
		// no action is necessary in this switch statement. See
		// ttetris_net_request.
		//
		switch (header->message_type) {
		case MSG_TYPE_GAME_STARTED:
			fprintf(logging_fp, "read_from_server: game started\n");
			blob = malloc(sizeof(Blob));
			blob->length = header->content_length;
			blob->bytes = cursor;
			StringArray *party_members =
			    string_array_deserialize(blob);
			render_init(party_members->length,
			            party_members->strings);
			free(blob);
			// signal that the game has started
			ttetris_event_mark_complete(
			    net_client->player->game_start_event);
			break;
		case MSG_TYPE_BOARD:
			read_game_view_data(cursor, net_client->player->view);
			break;
		case MSG_TYPE_REGISTER_SUCCESS:
		case MSG_TYPE_LIST_RESPONSE:
			break;
		default:
			// stop processing on this read chunk if it contained an
			// unknown message
			return EXIT_SUCCESS;
		}

		// if the message header has a non-zero request id, update the
		// local representation
		if (header->request_id != 0) {
			NetRequest *request;
			for (int i = 0; i < net_client->open_requests->length;
			     i++) {
				fprintf(logging_fp,
				        "checking for responses %d\n", i);
				request = (NetRequest *)list_get(
				    net_client->open_requests, i);
				if (request->id == header->request_id) {
					request->cursor =
					    malloc(header->content_length);
					memcpy(request->cursor, cursor,
					       header->content_length);
					ttetris_net_request_complete(request);
				}
			}
		}

		// increment the cursor by the message_size
		cursor += header->content_length;
	}

	return EXIT_SUCCESS;
}

void *tetris_thread(void *net_client) {
	while (read_from_server((NetClient *)net_client) == EXIT_SUCCESS) {
	}

	return 0;
}

/**
 * start listening to the server for updates to the board
 *
 * run this as a seperate thread
 */
void tetris_listen(NetClient *net_client) {
	if (net_client->is_listen_thread_started) {
		// prevent more than one thread from listening to the same
		// socket
		perror("cannot open a second listening thread");
		exit(EXIT_FAILURE);
	}

	int ret = pthread_create(&net_client->listen_thread, NULL,
	                         tetris_thread, net_client);
	if (ret) {
		perror("error creating thread");
		exit(EXIT_FAILURE);
	}
	net_client->is_listen_thread_started = 1;
}

NetClient *net_client_init() {
	fprintf(logging_fp, "net_client_init()\n");
	NetClient *net_client = malloc(sizeof(NetClient));
	net_client->is_listen_thread_started = 0;
	net_client->fd = -1;
	net_client->online_players = list_create();
	net_client->player = NULL;
	net_client->open_requests = list_create();
	return net_client;
};

NetRequest *ttetris_net_request(NetClient *client, char *bytes, uint16_t nbytes,
                                msg_type_t message_type) {
	NetRequest *request = malloc(sizeof(NetRequest));
	// TODO Stop using the number of requests sent as the ID. Of course, we
	// also want to be able to delete requests from this list at some point.
	request->id = client->open_requests->length + 1;
	request->response_event = ttetris_event_create();

	// IMPORTANT: list_append must be called before message_nbytes to avoid
	// a race condition
	list_append(client->open_requests, request);

	message_nbytes(client->fd, bytes, nbytes, request->id, message_type);

	return request;
};

void ttetris_net_request_block_for_response(NetRequest *request) {
	ttetris_event_block_for_completion(request->response_event);
}

// define a control set for use over TCP
static const TetrisControlSet TCPControlSet = {.translate = tetris_translate,
                                               .lower = tetris_lower,
                                               .rotate = tetris_rotate,
                                               .drop = tetris_drop,
                                               .swap_hold = tetris_swap_hold};

TetrisControlSet tcp_control_set(void) { return TCPControlSet; }
