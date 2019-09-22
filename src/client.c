#include <ncurses.h>

#include "client_conn.h"
#include "render.h"

#define PORT 5555
#define HOST "localhost"

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

  // connect to the server
  tetris_connect(HOST, PORT);

  render_init();
  render_board(board);

  // wait for ESC key
  char ch;
  while( (ch = getch()) != 27 ){
    switch(ch){
      case 'a':
        tetris_translate(-1, 0);
        break;
      case 's':
        tetris_translate(0, 1);
        break;
      case 'd':
        tetris_translate(1, 0);
        break;
      case 'q':
        tetris_rotate(-1);
        break;
      case 'e':
        tetris_rotate(1);
        break;
    }
  }

  // cleanup and exit
  render_close();
  tetris_disconnect(); 
  return 0;
}
