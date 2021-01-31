#ifndef _RENDER_HEADER
#define _RENDER_HEADER

// Before including curses, undefine MOUSE_MOVED if provided by windows.h
// TODO look for a more elegant solution
#undef MOUSE_MOVED
#include <curses.h>

#include "tetris_game.h"

void render_init(int n, char *names[]);

void render_close(void);

void render_game_view_data(char *board_name, struct game_view_data *view);

int print_centered(WINDOW *w, int y, char *text);

WINDOW *create_newwin(int height, int width, int starty, int startx);

void destroy_win(WINDOW *local_win);

void render_refresh_layout(void);

#endif
