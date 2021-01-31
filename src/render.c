#include <curses.h>
#include <locale.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "os_compat.h"
#ifdef THIS_IS_WINDOWS
#include <windows.h>
#include <winsock.h>
#else
#include <asm/ioctls.h>
#endif

#include "log.h"
#include "render.h"
#include "terminal_size.h"
#include "tetris_game.h"

// width of the windows used to show the points, lines, next block, and hold
// block
#define SECONDARY_WIN_WIDTH 8
#define CELL_WIDTH 2
#define BOARD_CH_WIDTH (CELL_WIDTH * BOARD_WIDTH)

struct board_display {
	char *name;
	WINDOW *tetris_window;
	WINDOW *next_block_window;
	WINDOW *hold_block_window;
	WINDOW *points_window;
	WINDOW *lines_window;
};

static struct board_display *boards = NULL;
static int nboards;
// flag used to indicate that the window layout needs to be refreshed
static int dirty;
// flags used to indicate the the root window is too small to render the
// contents
static int too_narrow = 0;
static int too_short = 0;

static struct board_display *board_from_name(char *name) {
	if (boards == NULL)
		return NULL;
	for (int i = 0; i < nboards; i++)
		if (strcmp(boards[i].name, name) == 0)
			return boards + i;
	return NULL;
}

/**
 * recalculate the positions of all tetris windows, etc
 */
void render_refresh_layout(void) { dirty = true; }

static void set_window(WINDOW **win, int height, int width, int starty,
                       int startx) {
	if (!*win) {
		*win = create_newwin(height, width, starty, startx);
	} else if (wresize(*win, height, width) != OK ||
	           mvwin(*win, starty, startx) != OK)
		exit(EXIT_FAILURE);
}

void _render_refresh_layout(void) {
	fprintf(logging_fp, "_render_refresh_layout nboards=%d\n", nboards);

	if (nboards == 0)
		return;
	//
	// There are a number of patterns to get ncurses to update the root
	// window size.
	// 1. Out of the box, ncurses should respond correctly to SIGWINCH and
	// upgetch KEY_RESIZE so that we can respond to
	//    the event.
	// 2. Calling endwin() followed by refresh() will force ncurses to
	// update. You would do this in a custom SIGWINCH
	//    handler.
	// 3. Calling resize_term directly with the output from ioctl
	//
	// In my testing, I found that approaches #1 and #2 were dropping the
	// next user keypress. Until we find a good solution to that, option #3
	// is working nicely. :)
	//
	TerminalSize term_size = get_terminal_size();
	// Here, we call the "inner" resize_term function rather than the
	// *recommended* "outer" resizeterm function. This is because in my
	// testing, resizeterm was causing the next user keypress to get
	// dropped, which is super annoying. There is probably another, better
	// solution, but this works fine. :)
	resize_term(term_size.rows, term_size.columns);

	clear();

	int board_min_y = (LINES - BOARD_HEIGHT) / 2;
	if (board_min_y < 1)
		board_min_y = 1;
	int panel_width = COLS / nboards;

	// check that the terminal is large enough to render everything
	too_narrow = panel_width < BOARD_CH_WIDTH + 10 ? 1 : 0;
	too_short = LINES < BOARD_HEIGHT + 2 ? 1 : 0;
	if (too_narrow || too_short) {
		refresh();
		return;
	}

	for (int i = 0; i < nboards; i++) {
		int panel_lowx = panel_width * i;
		int window_lowx =
		    panel_lowx +
		    (panel_width - BOARD_CH_WIDTH - SECONDARY_WIN_WIDTH) / 2;

		set_window(&boards[i].tetris_window, BOARD_HEIGHT + 2,
		           BOARD_CH_WIDTH + 2, board_min_y - 1,
		           window_lowx - 1);

		int secondary_start_x = window_lowx + BOARD_CH_WIDTH + 1;

		set_window(&boards[i].next_block_window, 6, SECONDARY_WIN_WIDTH,
		           board_min_y - 1, secondary_start_x);

		set_window(&boards[i].hold_block_window, 6, SECONDARY_WIN_WIDTH,
		           board_min_y + 5, secondary_start_x);

		set_window(&boards[i].points_window, 4, SECONDARY_WIN_WIDTH,
		           board_min_y + 11, secondary_start_x);

		set_window(&boards[i].lines_window, 4, SECONDARY_WIN_WIDTH,
		           board_min_y + 15, secondary_start_x);

		werase(boards[i].points_window);
		werase(boards[i].lines_window);

		// wnoutrefresh updates the virtual window, but not the
		// "physical window." The "physical window" is updated by the
		// doupdate() call below.
		wnoutrefresh(boards[i].next_block_window);
		wnoutrefresh(boards[i].hold_block_window);
		wnoutrefresh(boards[i].points_window);
		wnoutrefresh(boards[i].lines_window);
		wnoutrefresh(boards[i].tetris_window);
	}

	wnoutrefresh(stdscr);
	doupdate();
	dirty = false;
}

#ifdef THIS_IS_NOT_WINDOWS
/**
 * This signal handler is just slightly different than the one that comes with
 * ncurses.
 * - ncurses will override the ioctl window size with the environment variables
 * LINES or COLUMNS if they are set (See man resizeterm).
 * - ncurses will ungetch KEY_RESIZE afterwards, which this will not
 * @param sig
 */
static void render_handle_sig(int sig) { dirty = true; }

void (*cached_handler)(int);

/**
 * saves the current handler for signal to handler
 * @param signal
 * @param handler
 */
static void get_current_handler(int signal, void (**handler)(int)) {
	// struct sigaction query_action;
	// if (sigaction(signal, NULL, &query_action) < 0) {
	// 	// -1 indicates an error
	// 	*handler = NULL;
	// } else if (query_action.sa_handler == SIG_DFL) {
	// 	// default handler
	// 	*handler = NULL;
	// } else if (query_action.sa_handler == SIG_IGN) {
	// 	// signal ignored
	// 	*handler = NULL;
	// } else {
	// 	// a custom handler is defined and in effect
	// 	*handler = query_action.sa_handler;
	// }
	*handler = NULL;
}

/**
 * lightweight wrapper around the ncurses signal handler that sets the render
 * state to dirty
 *
 * Without this, continuing after a SIGTSTP will fail in messy ways, because
 * the separate tetris game engine thread will call render methods before the
 * render is set up
 */
static void render_handle_sigtstp(int sig) {
	dirty = true;
	if (cached_handler)
		cached_handler(sig);
}
#endif

/**
 * initialize the renderer to display n games for players given by the names in
 * the array names
 */
void render_init(int n, char *names[]) {
	fprintf(logging_fp, "render_init(n=%d, names=...)\n", n);
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
	init_pair(shadow, COLOR_CYAN, COLOR_CYAN);
	init_pair(orange, COLOR_RED, COLOR_RED);
	init_pair(blue, COLOR_BLUE, COLOR_GREEN);
	init_pair(cleve, COLOR_GREEN, COLOR_GREEN);
	init_pair(rhode, COLOR_YELLOW, COLOR_YELLOW);
	init_pair(teewee, COLOR_MAGENTA, COLOR_MAGENTA);
	init_pair(hero, COLOR_CYAN, COLOR_CYAN);
	init_pair(smashboy, COLOR_WHITE, COLOR_WHITE);

	// set the static variable for this module
	nboards = n;

	// allocate the boards array and set the names
	boards = calloc(sizeof(struct board_display), nboards);
	for (int i = 0; i < nboards; i++)
		boards[i].name = names[i];

	_render_refresh_layout();

#ifdef THIS_IS_NOT_WINDOWS
	signal(SIGWINCH, render_handle_sig);

	// cache the ncurses SIGTSTP handler so that we can call it right after
	// our stuff
	get_current_handler(SIGTSTP, &cached_handler);
	// set our handler for SIGTSTP
	signal(SIGTSTP, render_handle_sigtstp);
#endif
}

/**
 * cleanup everything associated with rendering the running game (ie. not menus,
 * or curses itself, etc)
 *
 * important: cleanup any other threads that might call the renderer before
 * using this method since this method will free memory
 */
void render_ingame_cleanup() {
	struct board_display *bd;

	for (int i = 0; i < nboards; i++) {
		bd = &boards[i];
		delwin(bd->tetris_window);
		delwin(bd->next_block_window);
		delwin(bd->hold_block_window);
		delwin(bd->points_window);
		delwin(bd->lines_window);
	}

	free(boards);
	boards = NULL;

	// clear the terminal
	clear();
	refresh();

	// end curses mode
	endwin();
}

void render_close(void) { render_ingame_cleanup(); }

/**
 * Render the terminal background for a coordinate pair in cell-space
 *
 * @param win   ncurses window
 * @param row   index of row
 * @param col   index of column
 * @param color color-pair index as passed to ncurses init_pair and used with
 *              the COLOR_PAIR macro
 * @param is_death_line any number other than 0 indicates that this is not part
 *              of the death line
 */
static void render_cell(WINDOW *win, int row, int col, short color,
                        int is_death_line) {
	int i;
	// render shadow block
	if (color == shadow) {
		// reset cell
		mvwchgat(win, 1 + row, 1 + col * CELL_WIDTH, CELL_WIDTH, 0, -1,
		         NULL);
		// handle narrow boards
		if (CELL_WIDTH == 1) {
			mvwaddch(win, 1 + row, 1 + col * CELL_WIDTH, '#');
		}
		// handle wide boards
		else {
			mvwaddch(win, 1 + row, 1 + col * CELL_WIDTH, '[');
			for (i = 2; i < CELL_WIDTH; i++) {
				mvwaddch(win, 1 + row, i + col * CELL_WIDTH,
				         ' ');
			}
			mvwaddch(win, 1 + row, i + col * CELL_WIDTH, ']');
		}
	}
	// render normal block
	else {
		for (i = 1; i <= CELL_WIDTH; i++)
			mvwaddch(win, 1 + row, i + col * CELL_WIDTH,
			         is_death_line ? '_' : ' ');
		mvwchgat(win, 1 + row, 1 + col * CELL_WIDTH, CELL_WIDTH, 0,
		         color, NULL);
	}
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
	struct position cell_offset;
	const struct position *offsets;
	int cell_count = get_tetris_block_offsets(&offsets, piece);
	if (cell_count <= 0)
		return;
	for (int i = 0; i < cell_count; i++) {
		cell_offset = rotate_position(offsets[i], rot);
		render_cell(win, pos.y - cell_offset.y, pos.x + cell_offset.x,
		            piece, 0);
	}
}

void render_game_view_data(char *name, struct game_view_data *view) {
	if (dirty)
		_render_refresh_layout();

	if (too_narrow) {
		print_centered(stdscr, 1, "Terminal too narrow!");
		print_centered(stdscr, 2, "Resize to play.");
		refresh();
		return;
	}
	if (too_short) {
		print_centered(stdscr, 1,
		               "Terminal too short! Resize to play.");
		refresh();
		return;
	}

	int(*board)[BOARD_WIDTH] = view->board;
	// figure out which board is getting rendered
	struct board_display *bd = board_from_name(name);

	if (bd == 0) {
		fprintf(logging_fp,
		        "render_game_view_data: no board found for name %s\n",
		        name);
		return;
	} else {
		fprintf(logging_fp,
		        "render_game_view_data: rendering board for %s\n",
		        name);
	}

	WINDOW *tetris_window = bd->tetris_window;

	// render the tetris window
	int r, c;
	for (r = 0; r < BOARD_HEIGHT; r++)
		for (c = 0; c < BOARD_WIDTH; c++)
			render_cell(tetris_window, BOARD_HEIGHT - r - 1, c,
			            board[r][c], r == BOARD_PLAY_HEIGHT);

	box(tetris_window, 0, 0);
	wnoutrefresh(tetris_window);

	// pre-render the next block window
	fill_window(bd->next_block_window, COLOR_PAIR(0));
	mvwprintw(bd->next_block_window, 1, 2, "NEXT");
	render_tetris_piece(bd->next_block_window, view->next_block, right,
	                    (struct position){1, 2});
	box(bd->next_block_window, 0, 0);
	wnoutrefresh(bd->next_block_window);

	// pre-render the hold block window
	fill_window(bd->hold_block_window, COLOR_PAIR(0));
	mvwprintw(bd->hold_block_window, 1, 2, "HOLD");
	render_tetris_piece(bd->hold_block_window, view->hold_block, right,
	                    (struct position){1, 2});
	box(bd->hold_block_window, 0, 0);
	wnoutrefresh(bd->hold_block_window);

	// pre-render the points window
	char output_buffer[64];
	sprintf(output_buffer, "POINTS\n %d", view->points);
	mvwprintw(bd->points_window, 1, 1, output_buffer);
	box(bd->points_window, 0, 0);
	wnoutrefresh(bd->points_window);

	// pre-render the lines window
	sprintf(output_buffer, "LINES\n %d", view->lines_cleared);
	mvwprintw(bd->lines_window, 1, 1, output_buffer);
	box(bd->lines_window, 0, 0);
	wnoutrefresh(bd->lines_window);

	wnoutrefresh(stdscr);
	// do update flushes all the window changes by wnoutrefresh at once
	doupdate();
}

/**
 * Make sure text is null terminated! Also, this function does not function
 * as expected if text contains non-printing characters.
 */
int print_centered(WINDOW *w, int y, char *text) {
	int max_y, max_x;
	getmaxyx(w, max_y, max_x);

	if (y > max_y) {
		return EXIT_FAILURE;
	}

	// if the text is too long, make a copy and null-terminate it
	char *output_buffer = text;
	int len = strnlen(text, max_x);
	if (len == max_x) {
		output_buffer = malloc(len + 1);
		memcpy(output_buffer, text, len);
		output_buffer[len] = 0;
	}

	// center x position
	int x = (max_x - len) / 2;

	mvwprintw(w, y, x, output_buffer);

	// free output_buffer if we made a copy of text
	if (output_buffer != text) {
		free(output_buffer);
	}

	return EXIT_SUCCESS;
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
