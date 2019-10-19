#include <errno.h>
#include <inttypes.h>
#include <ncurses.h>
#include <stdlib.h>

#include "client_conn.h"
#include "render.h"

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

void
usage()
{
  fprintf(stderr, "Usage: ./client ADDRESS PORT\n");
  exit(EXIT_FAILURE);
}

int
main(int argc, char * argv[])
{
  if( argc != 3)
    usage();

  char * host = argv[1];
  char * port = argv[2];

  uintmax_t numeric_port = strtoumax(port, NULL, 10);
  if (numeric_port == UINTMAX_MAX && errno == ERANGE) {
    fprintf(stderr, "Provided port is invalid\n");
    usage();
  }

  // connect to the server
  tetris_connect(host, numeric_port);

  render_init();
  render_board(board);

  tetris_listen(board);

  input_loop();

  // cleanup and exit
  render_close();
  tetris_disconnect();
  return 0;
}
