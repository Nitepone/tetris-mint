#include <ncurses.h>

#ifndef _RENDER_HEADER
#define _RENDER_HEADER

#define BOARD_WIDTH 10
#define BOARD_HEIGHT 24

void render_init(void);

void render_close(void);

void render_board(int board[BOARD_WIDTH][BOARD_HEIGHT]);

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *local_win);

#endif
