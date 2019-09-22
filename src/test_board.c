#include <ncurses.h>

#include "render.h"

// local cache of the user's tetris board
// global variables and static variables are automatically initialized to zero
int board[10][24];

int
main(void)
{
  board[5][17] = 1;
  board[6][17] = 1;
  board[7][17] = 1;
  board[6][16] = 1;


  render_init();
  render_board(board);

  // wait for 'q' key
  while( getch() != 'q' ){}

  render_close();

  return 0;
}
