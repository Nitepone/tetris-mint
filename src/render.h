#ifndef _RENDER_HEADER
#define _RENDER_HEADER

#include <ncurses.h>

#include "tetris_game.h"

void render_init(int n, char *names[]);

void render_close(void);

void render_board(char *name, int board[BOARD_HEIGHT][BOARD_WIDTH]);

void render_message(char *text);

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *local_win);

#endif
