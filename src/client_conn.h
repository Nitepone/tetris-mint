#ifndef _CLIENT_CONN_H
#define _CLIENT_CONN_H

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

#include "controller.h"
#include "player.h"

void tetris_send_message(char *message);

int tetris_connect(char *host, int port);

void tetris_disconnect();

void tetris_list();

void tetris_listen(Player *player);

void tetris_register(char *username);

void tetris_opponent(char *username);

TetrisControlSet tcp_control_set(void);

#endif
