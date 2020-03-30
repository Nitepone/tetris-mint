#include <locale.h>
#include <ncurses.h>
#include <stdlib.h>
#include <string.h>

#include "render.h"
#include "tetris_game.h"

#define CELL_WIDTH 2
#define BOARD_CH_WIDTH (CELL_WIDTH * BOARD_WIDTH)

struct board_display {
	char *name;
	WINDOW *tetris_window;
	WINDOW *message_window;
	WINDOW *top_window;
};

static struct board_display *boards;
static int nboards;

static struct board_display *board_from_name(char *name) {
	for (int i = 0; i < nboards; i++)
		if (strcmp(boards[i].name, name) == 0)
			return boards + i;
	return 0;
}

/**
 * initialize the renderer to display n games for players given by the names in
 * the array names
 */
void render_init(int n, char *names[]) {
	// If the locale is not initialized, the library assumes that characters
	// are printable as in ISO-8859-1, to work with certain legacy programs.
	// This is to be explicit.
	setlocale(LC_ALL, "ISO-8859-1");

	// initialize the curses system
	initscr();

	// disable line buffering, but allow control characters to still be
	// interpreted by the terminal emulator
	cbreak();

	// suppress echo of user entered characters
	noecho();

	// enable reading of function and arrow keys
	keypad(stdscr, TRUE);

	// hide the cursor
	curs_set(0);

	// start color functionality
	start_color();

	// create numbered fg/bg pairs to be used by the COLOR_PAIR macro later
	init_pair(0, COLOR_WHITE, COLOR_WHITE);
	init_pair(1, COLOR_CYAN, COLOR_CYAN);
	init_pair(2, COLOR_RED, COLOR_RED);
	init_pair(3, COLOR_GREEN, COLOR_GREEN);
	init_pair(4, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(5, COLOR_BLUE, COLOR_BLUE);
	init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(7, COLOR_CYAN, COLOR_CYAN);

	int starty = (LINES - BOARD_HEIGHT) / 2;

	// set the static variable for this module
	nboards = n;

	// allocate the boards array
	boards = malloc(n * sizeof(struct board_display));

	int panel_width = COLS / n;
	for (int i = 0; i < n; i++) {
		int panel_lowx = panel_width * i;
		int window_lowx =
		    panel_lowx + (panel_width - BOARD_CH_WIDTH) / 2;
		boards[i].name = names[i];
		boards[i].tetris_window =
		    create_newwin(BOARD_HEIGHT + 2, BOARD_CH_WIDTH + 2,
		                  starty - 1, window_lowx - 1);
		boards[i].message_window = create_newwin(
		    3, panel_width, starty + BOARD_HEIGHT + 2, panel_lowx);
		boards[i].top_window = create_newwin(
		    5, BOARD_CH_WIDTH + 2, starty - 7, window_lowx - 1);
	}

	refresh();
}

void render_close(void) {
	// close the window
	// destroy_win(tetris_window);
	// end curses mode
	endwin();
}

/**
 * Render the terminal background for a coordinate pair in cell-space
 *
 * @param win   ncurses window
 * @param row   index of row
 * @param col   index of column
 * @param color color-pair index as passed to ncurses init_pair and used with
 *              the COLOR_PAIR macro
 */
static void render_cell(WINDOW *win, int row, int col, short color) {
	for (int i = 1; i <= CELL_WIDTH; i++)
		mvwaddch(win, 1 + row, i + col * CELL_WIDTH, ' ');
	mvwchgat(win, 1 + row, 1 + col * CELL_WIDTH, CELL_WIDTH, 0, color,
	         NULL);
}

/**
 * Set the foreground and background color of an entine window
 *
 * NOTE: This is different than the bkgd family of functions in ncurses. This
 * function overrides any existing colors, even if they do not match the
 * previous color.
 *
 * @param win   ncurses window
 * @param color color-pair index as passed to ncurses init_pair and used with
 *              the COLOR_PAIR macro
 */
static void fill_window(WINDOW *win, short color) {
	int height, width;
	getmaxyx(stdscr, height, width);
	for (int row = 0; row < height; row++)
		mvwchgat(win, row, 0, width, 0, color, NULL);
}

static struct position rotate_position(struct position pos, enum rotation rot) {
	switch (rot) {
	case right:
		return (struct position){.x = pos.y, .y = -pos.x};
	case invert:
		return (struct position){.x = -pos.x, .y = -pos.y};
	case left:
		return (struct position){.x = -pos.y, .y = pos.x};
	case none:
	default:
		return pos;
	}
}

/**
 * render a tetris piece in a window
 *
 * IMPORTANT NOTE: The position for this function is given in ncurses space,
 * where the origin is the top left. The origin for the tetris library is bottom
 * left. As a result, this function flips the Y axis while rendering pieces.
 *
 * @param win   ncurses window
 * @param piece tetris piece
 * @param rot   tetris piece rotation
 * @param pos   position in ncurses space relative to the window
 */
static void render_tetris_piece(WINDOW *win, enum block_type piece,
                                enum rotation rot, struct position pos) {
	if (piece < 0 || piece >= BLOCK_TYPE_COUNT)
		return;
	struct position cell_offset;
	for (int i = 0; i < MAX_BLOCK_UNITS; i++) {
		cell_offset = rotate_position(block_offsets[piece][i], rot);
		render_cell(win, pos.y - cell_offset.y, pos.x + cell_offset.x,
		            piece + 1);
	}
}

void render_game_view_data(char *name, struct game_view_data *view) {
	int(*board)[BOARD_WIDTH] = view->board;
	// figure out which board is getting rendered
	struct board_display *bd = board_from_name(name);

	if (bd == 0) {
		printf("render_game_view_data: no board found for name\n");
		return;
	}

	WINDOW *tetris_window = bd->tetris_window;

	// render the tetris window
	int r, c;
	for (r = 0; r < BOARD_HEIGHT; r++)
		for (c = 0; c < BOARD_WIDTH; c++)
			render_cell(tetris_window, BOARD_HEIGHT - r - 1, c,
			            board[r][c]);

	box(tetris_window, 0, 0);
	wrefresh(tetris_window);

	char status[100];
	sprintf(status, "Points: %d   Lines: %d", view->points,
	        view->lines_cleared);

	// print the player's score beneath the board
	mvwprintw(bd->message_window, 1, 1, status);
	box(bd->message_window, 0, 0);
	wrefresh(bd->message_window);

	// print the next piece and swap piece in the top window
	fill_window(bd->top_window, COLOR_PAIR(0));
	render_tetris_piece(bd->top_window, view->next_block, right,
	                    (struct position){1, 1});
	render_tetris_piece(bd->top_window, view->hold_block, left,
	                    (struct position){BOARD_WIDTH - 2, 1});
	box(bd->top_window, 0, 0);
	wrefresh(bd->top_window);
}

///
// Warning: make sure text is properly null-terminated!
///
void render_message(char *text) {
	// center y position between bottom of board and bottom of terminal
	int y = (3 * LINES + BOARD_HEIGHT) / 4;
	// center x position
	int x = (COLS - strlen(text)) / 2;
	mvprintw(y, x, text);
}

WINDOW
*create_newwin(int height, int width, int starty, int startx) {
	WINDOW *local_win;
	local_win = newwin(height, width, starty, startx);
	box(local_win, 0, 0);
	wrefresh(local_win);

	return local_win;
}

void destroy_win(WINDOW *local_win) {
	wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
	wrefresh(local_win);
	delwin(local_win);
}
