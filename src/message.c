#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "player.h"
#include "message.h"


/**
 * Write n bytes to socket and return EXIT_SUCCESS or EXIT_FAILURE
 */
int
message_nbytes(int socket_fd, char * bytes, int n)
{
	int bytes_written = write (socket_fd, bytes, n);
	fprintf(stderr, "message_nbytes: sent %d bytes\n", bytes_written);

	if (bytes_written < 0) {
		perror ("write");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Send a list of all online users over the given socket.
 */
int
send_online_users (Player * player)
{
	int socket_fd = player->fd;
	StringArray * arr = player_names();
	Blob * blob = serialize_string_array(arr);
	shift_blob(blob, 3);
	*(uint16_t *)blob->bytes = blob->length;
	blob->bytes[2] = MSG_TYPE_LIST_RESPONSE;
	printf("send_online_users: message blob length: %d\n", blob->length);

	pthread_mutex_lock(&player->io_lock);
	message_nbytes(socket_fd, blob->bytes, blob->length);
	pthread_mutex_unlock(&player->io_lock);

	return EXIT_SUCCESS;
}

Blob *
serialize_state (Player * player)
{
	// first, render the board into the player view
	generate_game_view_data(&player->view, player->contents);

	// create a blob to contain the message
	Blob * blob = create_blob(MAXMSG);
	// first two bytes represent the length of the message
	*(uint16_t *)blob->bytes = blob->length;
	// next byte indicates message type
	blob->bytes[2] = MSG_TYPE_BOARD;
	// next null-terminated bytes are used to store the player name
	uint8_t name_length = strlen(player->name);
	memcpy(blob->bytes + 3, player->name, name_length + 1);
	// the board is sent directly after the null-byte
	memcpy(blob->bytes + 4 + name_length, player->view->board, 960);
	return blob;
}

/**
 * Send the board for the given player over the given socket.
 */
int
send_board (int socket_fd, struct st_player * player)
{
	if (socket_fd < 0) {
		fprintf(stderr, "send_board: skipping invalid file descriptor %d\n", socket_fd);
		return EXIT_FAILURE;
	}

	if (player == 0) {
		fprintf(stderr, "Error: No player found for socket\n");
		return EXIT_FAILURE;
	}

	if (player->view == 0) {
		fprintf(stderr, "Error: No view exists for player.\n");
		return EXIT_FAILURE;
	}

	if (player->view->board == 0) {
		fprintf(stderr, "Error: No board exists for player.\n");
		return EXIT_FAILURE;
	}

	Blob * blob = serialize_state(player);

	pthread_mutex_lock(&player->io_lock);
	int status = message_nbytes(socket_fd, blob->bytes, blob->length);
	pthread_mutex_unlock(&player->io_lock);

	return status;
}

// vi:noet:noai:sw=0:sts=0:ts=8
