#include <inttypes.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "client_conn.h"
#include "message.h"
#include "render.h"
#include "tetris_game.h"

// for backward compatibility
#define h_addr h_addr_list[0]

/**
 * Borrowed from GNU Socket Tutorial
 */
static void init_sockaddr(struct sockaddr_in *name, const char *hostname,
                          uint16_t port) {
	struct hostent *hostinfo;

	name->sin_family = AF_INET;
	name->sin_port = htons(port);
	hostinfo = gethostbyname(hostname);
	if (hostinfo == NULL) {
		fprintf(stderr, "Unknown host %s.\n", hostname);
		exit(EXIT_FAILURE);
	}
	name->sin_addr = *(struct in_addr *)hostinfo->h_addr;
}

void tetris_send_message(NetClient *net_client, char *body) {
	uint16_t len = strlen(body) + 1;
	ttetris_net_request(net_client, body, len);
}

/**
 * get socket or exit if an error occurs
 */
int get_socket() {
	int sock = socket(PF_INET, SOCK_STREAM, 0);
	if (sock >= 0)
		return sock;

	perror("socket (client)");
	exit(EXIT_FAILURE);
}

static void tetris_translate(void *net_client, int x) {
	char xdir = x > 0 ? 1 : 0;
	char message[128];
	sprintf(message, "%c%c", MSG_TYPE_TRANSLATE, xdir);
	tetris_send_message((NetClient *)net_client, message);
}

static void tetris_lower(void *net_client) {
	char message[8];
	sprintf(message, "%c", MSG_TYPE_LOWER);
	tetris_send_message((NetClient *)net_client, message);
}

static void tetris_rotate(void *net_client, int theta) {
	char dir = theta > 0 ? 1 : 0;
	char message[128];
	sprintf(message, "%c%c", MSG_TYPE_ROTATE, dir);
	tetris_send_message((NetClient *)net_client, message);
}

static void tetris_drop(void *net_client) {
	char message[128];
	sprintf(message, "%c", MSG_TYPE_DROP);
	tetris_send_message((NetClient *)net_client, message);
}

static void tetris_swap_hold(void *net_client) {
	char message[128];
	sprintf(message, "%c", MSG_TYPE_SWAP_HOLD);
	tetris_send_message((NetClient *)net_client, message);
}

StringArray *tetris_list(NetClient *net_client) {
	char message[1];
	message[0] = MSG_TYPE_LIST;
	NetRequest *request = ttetris_net_request(net_client, message, 1);
	ttetris_net_request_block_for_response(request);

	MessageHeader *header = (MessageHeader *)request->cursor;

	// deserialize names
	Blob *body = malloc(sizeof(Blob));
	body->bytes = request->cursor + 1;
	body->length = header->content_length;
	return string_array_deserialize(body);
}

/**
 * establish a connection to the server
 */
void tetris_connect(NetClient *net_client, char *host, int port) {
	struct sockaddr_in servername;

	/* Create the socket. */
	int sock_fd = get_socket();

	/* Connect to the server. */
	init_sockaddr(&servername, host, port);
	if (0 > connect(sock_fd, (struct sockaddr *)&servername,
	                sizeof(servername))) {
		perror("connect (client)");
		exit(EXIT_FAILURE);
	}

	net_client->fd = sock_fd;
}

/**
 * register
 */
void tetris_register(NetClient *net_client, char *username) {
	char message[128];
	sprintf(message, "%c%s", MSG_TYPE_REGISTER, username);
	tetris_send_message(net_client, message);
}

/**
 * select opponent by username
 */
void tetris_opponent(NetClient *net_client, char *username) {
	char message[128];
	sprintf(message, "%c%s", MSG_TYPE_OPPONENT, username);
	tetris_send_message(net_client, message);
}

void tetris_disconnect(NetClient *net_client) {
	close(net_client->fd);
	net_client->is_listen_thread_started = 0;
}

int read_game_view_data(char **cursor, struct game_view_data *view) {
	char *buffer = *cursor;
	// move the pointer past the message identifier
	++buffer;
	// get the player associated with the board
	char *board_name = buffer;
	int name_length = strlen(board_name);
	// move the pointer past the name string
	buffer += name_length + 1;
	// copy the game view data
	memcpy(view, buffer, sizeof(struct game_view_data));
	render_game_view_data(board_name, view);

	return EXIT_SUCCESS;
}

int read_names(char **cursor) {
	char *buffer = *cursor;
	int received_names = *(int *)(buffer + 1);
	printf("Num strings received: %d\n", received_names);
	char *current_name = buffer + 5;
	for (int i = 0; i < received_names; i++) {
		printf("Name: %s\n", current_name);
		current_name += strlen(current_name) + 1;
	}

	return EXIT_SUCCESS;
}

void ttetris_net_request_complete(NetRequest *request) {
	pthread_mutex_lock(&request->ready_mutex);
	request->ready_flag = 1;
	pthread_cond_broadcast(&request->ready_cond);
	pthread_mutex_unlock(&request->ready_mutex);
};

int read_from_server(NetClient *net_client) {
	char buffer[MAXMSG];

	// remember that more than one TCP packet may be read by this command
	int nbytes = read(net_client->fd, buffer, MAXMSG);

	// exit early if there was an error
	if (nbytes < 0) {
		perror("read");
		return EXIT_FAILURE;
	}

	// exit early if we reached the end-of-file
	if (nbytes == 0)
		return -1;

	// initialize pointers for moving through data
	char *end = buffer + nbytes;
	char *cursor = buffer;

	while (cursor < end) {
		MessageHeader *header = (MessageHeader *)buffer;

		// increment the cursor to the start of the message body
		cursor += sizeof(MessageHeader);

		switch (cursor[0]) {
		case MSG_TYPE_BOARD:
			read_game_view_data(&cursor, net_client->player->view);
			break;
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
				request = (NetRequest *)list_get(
				              net_client->open_requests, i)
				              ->target;
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
	NetClient *net_client = malloc(sizeof(NetClient));
	net_client->is_listen_thread_started = 0;
	net_client->fd = -1;
	net_client->online_players = list_create();
	net_client->player = NULL;
	net_client->open_requests = list_create();
	return net_client;
};

NetRequest *ttetris_net_request(NetClient *client, char *bytes,
                                u_int16_t nbytes) {
	NetRequest *request = malloc(sizeof(NetRequest));
	// TODO Stop using the number of requests sent as the ID. Of course, we
	// also want to be able to delete requests from this list at some point.
	request->id = client->online_players->length + 1;
	request->ready_flag = 0;
	pthread_cond_init(&request->ready_cond, NULL);
	pthread_mutex_init(&request->ready_mutex, NULL);

	message_nbytes(client->fd, bytes, nbytes, request->id);

	list_append(client->open_requests, request);

	return request;
};

void ttetris_net_request_block_for_response(NetRequest *request) {
	pthread_mutex_lock(&request->ready_mutex);
	while (!request->ready_flag) {
		pthread_cond_wait(&request->ready_cond, &request->ready_mutex);
	}
	pthread_mutex_unlock(&request->ready_mutex);
}

// define a control set for use over TCP
static const TetrisControlSet TCPControlSet = {.translate = tetris_translate,
                                               .lower = tetris_lower,
                                               .rotate = tetris_rotate,
                                               .drop = tetris_drop,
                                               .swap_hold = tetris_swap_hold};

TetrisControlSet tcp_control_set(void) { return TCPControlSet; }
