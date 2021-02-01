#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "os_compat.h"
#ifdef THIS_IS_WINDOWS
#include <winsock2.h>
#else
#include <sys/socket.h>
#endif

#include "log.h"
#include "message.h"
#include "player.h"
#include "tetris_game.h"

char *message_type_to_str(msg_type_t msg_type) {
	switch (msg_type) {
	case MSG_TYPE_UNKNOWN:
		return "UNKNOWN";
	case MSG_TYPE_REGISTER:
		return "REGISTER";
	case MSG_TYPE_REGISTER_SUCCESS:
		return "REGISTER_SUCCESS";
	case MSG_TYPE_OPPONENT:
		return "OPPONENT";
	case MSG_TYPE_ROTATE:
		return "ROTATE";
	case MSG_TYPE_TRANSLATE:
		return "TRANSLATE";
	case MSG_TYPE_LOWER:
		return "LOWER";
	case MSG_TYPE_DROP:
		return "DROP";
	case MSG_TYPE_SWAP_HOLD:
		return "SWAP_HOLD";
	case MSG_TYPE_LIST:
		return "LIST";
	case MSG_TYPE_BOARD:
		return "BOARD";
	case MSG_TYPE_LIST_RESPONSE:
		return "LIST_RESPONSE";
	case MSG_TYPE_START_GAME:
		return "START_GAME";
	case MSG_TYPE_GAME_STARTED:
		return "GAME_STARTED";
	default:
		return "UNKNOWN";
	}
}

int message_nbytes(SOCKET socket_fd, char *bytes, int nbytes, int request_id,
                   msg_type_t message_type) {
	unsigned long payload_bytes = sizeof(MessageHeader) + nbytes;
	char *payload = calloc(sizeof(char), payload_bytes);

	// write the payload header
	MessageHeader *header = (MessageHeader *)payload;
	header->magic_number = MSG_MAGIC_NUMBER;
	header->content_length = nbytes;
	header->request_id = request_id;
	header->message_type = message_type;

	// copy the body into the payload
	memcpy(payload + sizeof(MessageHeader), bytes, nbytes);

	int bytes_written = send(socket_fd, payload, payload_bytes, 0);

	free(payload);

	// Note: Windows uses a 64-bit socket number, but Linux uses a 32-bit
	// number. For now, this will just print the lower 32 bits.
	fprintf(logging_fp,
	        "message_nbytes: Wrote %d bytes to file pointer %x, "
	        "content_length=%d request_id=%d message_type=%s \n",
	        bytes_written, (uint32_t)(socket_fd & 0xFFFF), nbytes,
	        request_id, message_type_to_str(message_type));

#ifdef THIS_IS_WINDOWS
	if (bytes_written == SOCKET_ERROR) {
#else
	if (bytes_written < 0) {
#endif
		fprintf(logging_fp,
		        "message_nbytes: socket error during write");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int message_blob(SOCKET socket_fd, Blob *blob, int request_id,
                 msg_type_t message_type) {
	return message_nbytes(socket_fd, blob->bytes, blob->length, request_id,
	                      message_type);
}

/**
 * Send a list of all online users over the given socket.
 */
int send_online_users(int filedes, int request_id) {
	StringArray *arr = player_names(1);
	Blob *blob = string_array_serialize(arr);
	message_blob(filedes, blob, request_id, MSG_TYPE_LIST_RESPONSE);
	return EXIT_SUCCESS;
}

Blob *serialize_state(Player *player) {
	// first, render the board into the player view
	generate_game_view_data(&player->view, player->contents);
	// figure out how big our blob needs to be
	uint8_t name_length = strnlen(player->name, PLAYER_NAME_MAX_CHARS);
	uint16_t blob_size = sizeof(struct game_view_data) + name_length + 1;
	// create a blob to contain the message
	Blob *blob = create_blob(blob_size);
	// next null-terminated bytes are used to store the player name
	strncpy(blob->bytes, player->name, name_length + 1);
	// strncpy will not null-terminate the string if it is longer than n,
	// so this will keep us safe
	blob->bytes[name_length] = 0;
	// the game_view_data is sent directly after the null-byte
	memcpy(blob->bytes + 1 + name_length, player->view,
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

	return message_blob(socket_fd, blob, 0, MSG_TYPE_BOARD);
}

// vi:noet:noai:sw=0:sts=0:ts=8
