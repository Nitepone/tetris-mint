#ifndef MESSAGE_H
#define MESSAGE_H

#include "player.h"
#include "tetris_game.h"

// the message must be able to hold a 4-byte integer for each cell in the
// board, and must have additional space for metadata (such as the player's
// name)
#define MAXMSG (4 * BOARD_WIDTH * BOARD_HEIGHT + 256)

#define MSG_TYPE_REGISTER 'U'
#define MSG_TYPE_OPPONENT 'O'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'
#define MSG_TYPE_DROP 'D'
#define MSG_TYPE_LIST 'P'
#define MSG_TYPE_BOARD 'B'
#define MSG_TYPE_LIST_RESPONSE 'Y'

int message_nbytes(int socket_fd, char *bytes, int n);

int send_online_users(Player *player);

int send_player(int socket_fd, Player *player);

#endif
