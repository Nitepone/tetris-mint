#include <locale.h>
#include <ncurses.h>
#include <string.h>

#include "render.h"

#define CELL_WIDTH 2
#define BOARD_CH_WIDTH (CELL_WIDTH * BOARD_WIDTH)

static WINDOW * tetris_window;

void
render_init(void)
{
  // If the locale is not initialized, the library assumes that characters are
  // printable as in ISO-8859-1, to work with certain legacy programs. This is
  // to be explicit.
  setlocale(LC_ALL, "ISO-8859-1");

  // initialize the curses system
  initscr();

  // disable line buffering, but allow control characters to still be
  // interpreted by the terminal emulator
  cbreak();

  // suppress echo of user entered characters
  noecho();

  // hide the cursor
  curs_set(0);

  // start color functionality
  start_color();

  // create numbered fg/bg pairs to be used by the COLOR_PAIR macro later
  init_pair(1, COLOR_CYAN, COLOR_CYAN);
  init_pair(2, COLOR_RED, COLOR_RED);
  init_pair(3, COLOR_GREEN, COLOR_GREEN);
  init_pair(4, COLOR_YELLOW, COLOR_YELLOW);
  init_pair(5, COLOR_BLUE, COLOR_BLUE);
  init_pair(6, COLOR_MAGENTA, COLOR_MAGENTA);
  init_pair(7, COLOR_CYAN, COLOR_CYAN);

  int starty = (LINES - BOARD_HEIGHT) / 2;
  int startx = (COLS - BOARD_CH_WIDTH) / 2;

  refresh();
  tetris_window = create_newwin(BOARD_HEIGHT + 2, BOARD_CH_WIDTH + 2, starty - 1, startx - 1);
}

void
render_close(void)
{
  // close the window
  destroy_win(tetris_window);
  // end curses mode
  endwin();
}

void
render_board(int board[BOARD_HEIGHT][BOARD_WIDTH])
{
  int r, c;
  for (r=0;r<BOARD_HEIGHT;r++)
    for (c=0;c<BOARD_WIDTH;c++)
      if (board[r][c])
        mvwchgat(tetris_window, 1 + r, 1 + c * CELL_WIDTH, CELL_WIDTH, 0, board[r][c], NULL);
  wrefresh(tetris_window);
}

///
// Warning: make sure text is properly null-terminated!
///
void
render_message(char * text)
{
  // center y position between bottom of board and bottom of terminal
  int y = (3 * LINES + BOARD_HEIGHT) / 4;
  // center x position
  int x = (COLS - strlen(text)) / 2;
  mvprintw(y, x, text);
}


WINDOW
*create_newwin(int height, int width, int starty, int startx)
{
  WINDOW *local_win;
  local_win = newwin(height, width, starty, startx);
  box(local_win, 0, 0);
  wrefresh(local_win);

  return local_win;
}

void
destroy_win(WINDOW *local_win)
{
  wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
  wrefresh(local_win);
  delwin(local_win);
}
