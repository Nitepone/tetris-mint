#ifndef MESSAGE_H
#define MESSAGE_H

#include "player.h"
#include "tetris_game.h"

typedef u_int8_t msg_type_t;

#define MSG_MAGIC_NUMBER 0xfeedU

// the message must be able to hold a 4-byte integer for each cell in the
// board, and must have additional space for metadata (such as the player's
// name)
#define MAXMSG 2048

// MSG_TYPE_UNKNOWN should be avoided when possible, but is used to indicate
// any special message that does not conform to one of the standard message
// types
#define MSG_TYPE_UNKNOWN 0
#define MSG_TYPE_REGISTER 'U'
// MSG_TYPE_REGISTER_SUCCESS is sent by the server when a user is successfully
// registered
#define MSG_TYPE_REGISTER_SUCCESS 'V'
#define MSG_TYPE_OPPONENT 'O'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'
#define MSG_TYPE_DROP 'D'
#define MSG_TYPE_SWAP_HOLD 'S'
#define MSG_TYPE_LIST 'P'
#define MSG_TYPE_BOARD 'B'
#define MSG_TYPE_LIST_RESPONSE 'Y'
// MSG_TYPE_START_GAME is sent from a client to the server to request that the
// game should begin
#define MSG_TYPE_START_GAME 'A'
// MSG_TYPE_GAME_STARTED is sent from the server to clients when the game has
// been started
#define MSG_TYPE_GAME_STARTED 'C'

typedef struct ttetris_msg_header MessageHeader;

struct ttetris_msg_header {
	/* magic number used to detect if our reader is mis-aligned and
	 * potentially avoid errors */
	u_int32_t magic_number;
	/* id to correlate messages, set to 0 if not needed */
	u_int16_t request_id;
	/* (required) length of the message body (not including the header) */
	u_int16_t content_length;
	/* (optional) type of message being sent */
	msg_type_t message_type;
};

/**
 * Get a textual representation of a message type
 *
 * Intended mostly for debugging and logging purposes
 * @param msg_type
 * @return
 */
char *message_type_to_str(msg_type_t msg_type);

/**
 * Write n bytes to socket and return EXIT_SUCCESS or EXIT_FAILURE
 */
int message_nbytes(int socket_fd, char *bytes, int n, int request_id,
                   msg_type_t message_type);

/**
 * Wrapper for message_nbytes that takes a blob
 */
int message_blob(int socket_fd, Blob *blob, int request_id,
                 msg_type_t message_type);

int send_online_users(int filedes, int request_id);

int send_player(int socket_fd, Player *player);

#endif
