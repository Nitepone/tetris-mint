#include <ncurses.h>

#include "client_conn.h"
#include "render.h"

#define PORT 5555
#define HOST "localhost"

// local cache of the user's tetris board
// global variables and static variables are automatically initialized to zero
int board[BOARD_HEIGHT][BOARD_WIDTH];

///
// Loop and handle input until ESC is pressed
///
void
input_loop()
{
  char ch;
  while( (ch = getch()) != 27 ){
    switch(ch){
      case 'a':
        tetris_translate(-1, 0);
        break;
      case 's':
        tetris_drop();
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
}

int
main(void)
{
  // connect to the server
  tetris_connect(HOST, PORT);

  render_init();
  render_board(board);

  tetris_listen(board);

  input_loop();

  // cleanup and exit
  render_close();
  tetris_disconnect();
  return 0;
}
