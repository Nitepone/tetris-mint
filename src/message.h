#ifndef MESSAGE_H
#define MESSAGE_H

#define MAXMSG 1280
#define MSG_TYPE_REGISTER 'U'
#define MSG_TYPE_OPPONENT 'O'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'
#define MSG_TYPE_DROP 'D'

int message_nbytes(int socket_fd, char * bytes, int n);

int send_board (int socket_fd, struct st_player * player);

#endif
