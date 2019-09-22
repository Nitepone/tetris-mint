#include <locale.h>
#include <ncurses.h>

#include "render.h"

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
  init_pair(1, COLOR_CYAN, COLOR_BLACK);
  init_pair(2, COLOR_RED, COLOR_BLACK);
  init_pair(3, COLOR_GREEN, COLOR_BLACK);
  init_pair(4, COLOR_YELLOW, COLOR_BLACK);
  init_pair(5, COLOR_BLUE, COLOR_BLACK);
  init_pair(6, COLOR_MAGENTA, COLOR_BLACK);
  init_pair(7, COLOR_CYAN, COLOR_BLACK);

  int starty = (LINES - BOARD_HEIGHT) / 2;
  int startx = (COLS - BOARD_WIDTH) / 2;

  refresh();
  tetris_window = create_newwin(BOARD_HEIGHT, BOARD_WIDTH, starty, startx);
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
render_board(int board[10][24])
{
  int r, c;
  for (r=0;r<10;r++)
    for (c=0;c<24;c++)
      if (board[r][c])
        mvwaddch(tetris_window, c, r, ACS_BLOCK | COLOR_PAIR(board[c][r]));
  wrefresh(tetris_window);
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
