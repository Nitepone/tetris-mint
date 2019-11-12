#include <errno.h>
#include <inttypes.h>
#include <ncurses.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

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
        tetris_translate(-1);
        break;
      case 'w':
        tetris_drop();
        break;
      case 's':
        tetris_lower(1);
        break;
      case 'd':
        tetris_translate(1);
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
  fprintf(stderr, "Usage: ./client [-h] [-u USERNAME] [-o OPPONENT] [-a ADDRESS] [-p PORT]\n");
  exit(EXIT_FAILURE);
}

int
main(int argc, char * argv[])
{
  char username[32] = "Elliot";
  char opponent[32] = "";
  char host[128] = "127.0.0.1";
  char port[6] = "5555";

  int list_players = 0;

  // Parse command line flags. The optstring passed to getopt has a preceding
  // colon to tell getopt that missing flag values should be treated
  // differently than unknown flags. The proceding colons indicate that flags
  // must have a value.
  int opt;
  while((opt = getopt(argc, argv, ":hlu:o:a:p:")) != -1)
  {
    switch(opt)
    {
    case 'h':
      usage();
      break;
    case 'u':
      strncpy(username, optarg, 31);
      printf("username: %s\n", optarg);
      break;
    case 'o':
      strncpy(opponent, optarg, 31);
      printf("opponent: %s\n", optarg);
      break;
    case 'a':
      strncpy(host, optarg, 127);
      printf("address: %s\n", optarg);
      break;
    case 'p':
      strncpy(port, optarg, 5);
      printf("port: %s\n", optarg);
      break;
    case 'l':
      list_players = 1;
      break;
    case ':':
      printf("option -%c needs a value\n", optopt);
      break;
    case '?':
      printf("unknown option: %c\n", optopt);
      break;
    }
  }

  // convert the string port to a number port
  uintmax_t numeric_port = strtoumax(port, NULL, 10);
  if (numeric_port == UINTMAX_MAX && errno == ERANGE) {
    fprintf(stderr, "Provided port is invalid\n");
    usage();
  }

  // connect to the server
  tetris_connect(host, numeric_port);
  // tetris_listen(board);
  // tetris_register(username);
  // tetris_list();
  // input_loop();

  // return 0;
  tetris_register(username);
  tetris_opponent(opponent);


  char * names[2];
  names[0] = username;
  names[1] = opponent;


  // render two blank boards
  render_board(username, board);
  render_board(opponent, board);

  tetris_listen(board);

  if (list_players) {
    tetris_list();
    sleep(1);
  } else {
    render_init(2, names);
    input_loop();
    render_close();
  }

  tetris_disconnect();
  return 0;
}
