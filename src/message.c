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

	char message[MAXMSG] = "BOARD";
	// sixth byte represents the length of the player's name
	uint8_t name_length = strlen(player->name);
	message[5] = name_length;
	// the next name_length + 1 bytes contain the player's name followed by
	// the null byte
	memcpy(message + 6, player->name, name_length + 1);

	// the board is sent at position 7 + name_length
	generate_game_view_data(&player->view, player->contents);
	memcpy(message + 7 + name_length, player->view->board, 960);

	// send the message
	return message_nbytes(socket_fd, message, 967 + name_length);
}

// vi:noet:noai:sw=0:sts=0:ts=8
