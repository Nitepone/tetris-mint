#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "message.h"
#include "player.h"
#include "tetris_game.h"

/**
 * Write n bytes to socket and return EXIT_SUCCESS or EXIT_FAILURE
 */
int message_nbytes(int socket_fd, char *bytes, int nbytes, int request_id) {
	int payload_bytes = sizeof(MessageHeader) + nbytes;

	char payload[payload_bytes];

	// write the payload header
	MessageHeader *header = (MessageHeader *)payload;
	header->content_length = nbytes;
	header->request_id = request_id;

	// copy the body into the payload
	memcpy(payload + sizeof(MessageHeader), bytes, nbytes);

	int bytes_written = write(socket_fd, payload, payload_bytes);

	if (bytes_written < 0) {
		perror("write");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

/**
 * Send a list of all online users over the given socket.
 */
int send_online_users(int filedes, int request_id) {
	StringArray *arr = player_names();
	Blob *blob = string_array_serialize(arr);
	shift_blob(blob, 1);
	blob->bytes[0] = MSG_TYPE_LIST_RESPONSE;
	printf("send_online_users: message blob length: %d\n", blob->length);

	message_nbytes(filedes, blob->bytes, blob->length, request_id);

	return EXIT_SUCCESS;
}

Blob *serialize_state(Player *player) {
	// first, render the board into the player view
	generate_game_view_data(&player->view, player->contents);

	// create a blob to contain the message
	Blob *blob = create_blob(MAXMSG);
	// first two bytes represent the length of the message
	// next byte indicates message type
	blob->bytes[0] = MSG_TYPE_BOARD;
	// next null-terminated bytes are used to store the player name
	uint8_t name_length = strlen(player->name);
	memcpy(blob->bytes + 1, player->name, name_length + 1);
	// the game_view_data is sent directly after the null-byte
	memcpy(blob->bytes + 2 + name_length, player->view,
	       sizeof(struct game_view_data));
	return blob;
}

/**
 * Send information about the given player, such as the name and game view data
 */
int send_player(int socket_fd, struct st_player *player) {
	if (socket_fd < 0) {
		fprintf(stderr,
		        "send_player: skipping invalid file descriptor %d\n",
		        socket_fd);
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

	Blob *blob = serialize_state(player);

	pthread_mutex_lock(&player->io_lock);
	int status = message_nbytes(socket_fd, blob->bytes, blob->length, 0);
	pthread_mutex_unlock(&player->io_lock);

	return status;
}

// vi:noet:noai:sw=0:sts=0:ts=8
