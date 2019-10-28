#include <ncurses.h>

#include "render.h"

// local cache of the user's tetris board
// global variables and static variables are automatically initialized to zero
int board[BOARD_HEIGHT][BOARD_WIDTH];

int
main(void)
{
  // T piece
  board[17][5] = 1;
  board[17][6] = 1;
  board[17][7] = 1;
  board[16][6] = 1;

  // L piece
  board[21][0] = 3;
  board[22][0] = 3;
  board[23][0] = 3;
  board[23][1] = 3;

  // line piece
  board[23][2] = 4;
  board[23][3] = 4;
  board[23][4] = 4;
  board[23][5] = 4;

  char * username = "Elliot";
  char * names[1];
  names[0] = username;

  render_init(1, names);
  render_board(username, board);
  render_message("Welcome to Tetris!");

  // wait for 'q' key
  while( getch() != 'q' ){}

  render_close();

  return 0;
}
