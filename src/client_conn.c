#include <errno.h>
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
#include "generic.h"
#include "message.h"
#include "render.h"
#include "tetris_game.h"

// for backward compatibility
#define h_addr h_addr_list[0]

// socket to communicate with server
static int sock_fd = -1;
pthread_t listen_thread;

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

void tetris_send_message(char *body) {
	uint16_t len = strlen(body) + 1;
	char message[128];
	message[0] = len >> 8;
	message[1] = len & 0xFF;
	memcpy(message + 2, body, len);
	int nbytes = write(sock_fd, message, len + 2);
	if (nbytes < 0) {
		perror("write");
		exit(EXIT_FAILURE);
	}
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

static void tetris_translate(int x) {
	char xdir = x > 0 ? 1 : 0;
	char message[128];
	sprintf(message, "%c%c", MSG_TYPE_TRANSLATE, xdir);
	tetris_send_message(message);
}

static void tetris_lower() {
	char message[8];
	sprintf(message, "%c", MSG_TYPE_LOWER);
	tetris_send_message(message);
}

static void tetris_rotate(int theta) {
	char dir = theta > 0 ? 1 : 0;
	char message[128];
	sprintf(message, "%c%c", MSG_TYPE_ROTATE, dir);
	tetris_send_message(message);
}

static void tetris_drop() {
	char message[128];
	sprintf(message, "%c", MSG_TYPE_DROP);
	tetris_send_message(message);
}

static void tetris_swap_hold() {
	char message[128];
	sprintf(message, "%c", MSG_TYPE_SWAP_HOLD);
	tetris_send_message(message);
	return;
}

void tetris_list() {
	char message[128];
	sprintf(message, "%c", MSG_TYPE_LIST);
	tetris_send_message(message);
}

/**
 * establish a connection to the server
 */
int tetris_connect(char *host, int port) {
	struct sockaddr_in servername;

	/* Create the socket. */
	sock_fd = get_socket();

	/* Connect to the server. */
	init_sockaddr(&servername, host, port);
	if (0 > connect(sock_fd, (struct sockaddr *)&servername,
	                sizeof(servername))) {
		perror("connect (client)");
		exit(EXIT_FAILURE);
	}

	return sock_fd;
}

/**
 * register
 */
void tetris_register(char *username) {
	char message[128];
	sprintf(message, "%c%s", MSG_TYPE_REGISTER, username);
	tetris_send_message(message);
}

/**
 * select opponent by username
 */
void tetris_opponent(char *username) {
	char message[128];
	sprintf(message, "%c%s", MSG_TYPE_OPPONENT, username);
	tetris_send_message(message);
}

void tetris_disconnect() { close(sock_fd); }

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

int read_from_server(Player *player) {
	char buffer[MAXMSG];

	// remember that more than one TCP packet may be read by this command
	int nbytes = read(player->fd, buffer, MAXMSG);

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
		// the first two bytes of the message should be used to indicate
		// the remaining number of bytes in the message
		uint16_t message_size = (buffer[0] << 8) + buffer[1];

		// increment the cursor to the start of the message body
		cursor += 2;

		switch (cursor[0]) {
		case MSG_TYPE_BOARD:
			read_game_view_data(&cursor, player->view);
			break;
		case MSG_TYPE_LIST_RESPONSE:
			read_names(&cursor);
			break;
		default:
			// stop processing on this read chunk if it contained an
			// unknown message
			return EXIT_SUCCESS;
		}

		// increment the cursor by the message_size
		cursor += message_size;
	}

	return EXIT_SUCCESS;
}

void *tetris_thread(void *player) {
	while (read_from_server((Player *)player) == EXIT_SUCCESS) {
	}

	return 0;
}

/**
 * start listening to the server for updates to the board
 *
 * run this as a seperate thread
 */
void tetris_listen(Player *player) {
	pthread_create(&listen_thread, NULL, tetris_thread, player);
}

// define a control set for use over TCP
static const TetrisControlSet TCPControlSet = {.translate = tetris_translate,
                                               .lower = tetris_lower,
                                               .rotate = tetris_rotate,
                                               .drop = tetris_drop,
                                               .swap_hold = tetris_swap_hold};

TetrisControlSet tcp_control_set(void) { return TCPControlSet; }
