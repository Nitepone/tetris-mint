#ifndef MESSAGE_H
#define MESSAGE_H

#include "player.h"

#define MAXMSG 1280
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

int send_board(int socket_fd, Player *player);

#endif
