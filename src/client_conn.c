#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <string.h>
#include <netdb.h>
#include <inttypes.h>

#include "client_conn.h"
#include "render.h"

#define MSG_TYPE_REGISTER 'U'
#define MSG_TYPE_OPPONENT 'O'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'
#define MSG_TYPE_DROP 'D'

// for backward compatibility
#define h_addr h_addr_list[0]

// socket to communicate with server
static int sock_fd = -1;
struct sockaddr_in servername;
pthread_t listen_thread;


/**
 * Borrowed from GNU Socket Tutorial
 */
static void
init_sockaddr (struct sockaddr_in *name,
               const char *hostname,
               uint16_t port)
{
  struct hostent *hostinfo;

  name->sin_family = AF_INET;
  name->sin_port = htons (port);
  hostinfo = gethostbyname (hostname);
  if (hostinfo == NULL)
    {
      fprintf (stderr, "Unknown host %s.\n", hostname);
      exit (EXIT_FAILURE);
    }
  name->sin_addr = *(struct in_addr *) hostinfo->h_addr;
}

void
tetris_send_message (char * body)
{
  uint16_t len = strlen (body) + 1;
  char message[128];
  message[0] = len >> 8;
  message[1] = len & 0xFF;
  memcpy(message + 2, body, len);
  int nbytes = write (sock_fd, message, len + 2);
  if (nbytes < 0) {
    perror ("write");
    exit (EXIT_FAILURE);
  }
}


/**
 * get socket or exit if an error occurs
 */
int
get_socket()
{
  int sock = socket (PF_INET, SOCK_STREAM, 0);
  if (sock >= 0)
    return sock;

  perror ("socket (client)");
  exit (EXIT_FAILURE);
}

void
tetris_translate(int x)
{
  char xdir = x > 0 ? 1 : 0;
  char message[128];
  sprintf(message, "%c%c", MSG_TYPE_TRANSLATE, xdir);
  tetris_send_message(message);
}

void
tetris_lower()
{
  char message[8];
  sprintf(message, "%c", MSG_TYPE_LOWER);
  tetris_send_message(message);
}

void
tetris_rotate(int theta)
{
  char dir = theta > 0 ? 1 : 0;
  char message[128];
  sprintf(message, "%c%c", MSG_TYPE_ROTATE, dir);
  tetris_send_message(message);
}

void
tetris_drop()
{
  char message[128];
  sprintf(message, "%c", MSG_TYPE_DROP);
  tetris_send_message(message);
}

/**
 * establish a connection to the server
 */
void
tetris_connect(char * host, int port)
{
  /* Create the socket. */
  sock_fd = get_socket();

  /* Connect to the server. */
  init_sockaddr (&servername, host, port);
  if (0 > connect (sock_fd,
                   (struct sockaddr *) &servername,
                   sizeof (servername)))
    {
      perror ("connect (client)");
      exit (EXIT_FAILURE);
    }
}

/**
 * register
 */
void
tetris_register(char * username)
{
  char message[128];
  sprintf(message, "%c%s", MSG_TYPE_REGISTER, username);
  tetris_send_message(message);
}

/**
 * select opponent by username
 */
void
tetris_opponent(char * username)
{
  char message[128];
  sprintf(message, "%c%s", MSG_TYPE_OPPONENT, username);
  tetris_send_message(message);
}

void
tetris_disconnect()
{
  close(sock_fd);
}

void *
tetris_thread(void * board_ptr)
{
  // 1024 bytes is enough for the whole board
  char buffer[1024];
  char message[256];
  int read_bytes = 0;
  int (*board)[BOARD_WIDTH] = board_ptr;
  while( (read_bytes = read(sock_fd, buffer, 1024)) > 0 ){
    sprintf(message, "Received %d bytes from server, starting with %c%c%c%c%c", read_bytes, buffer[0], buffer[1], buffer[2], buffer[3],buffer[4]);
    if (memcmp("BOARD", buffer, 5) == 0) {
      sprintf(message, "Received board from server");
      memcpy(board, buffer + 5, 960);
      for (int i=0;i<960;i++)
        if( *(buffer + 5 + i) != 0 )
          sprintf(message, "Cell %d is set", i);

      if (board[20][5] == 1){
        render_message("piece set");
      }
      render_board(board);
    }
    render_message(message);
  }
  return 0;
}

/**
 * start listening to the server for updates to the board
 *
 * run this as a seperate thread
 */
void
tetris_listen(int board[BOARD_HEIGHT][BOARD_WIDTH])
{
  pthread_create(&listen_thread, NULL, tetris_thread, board);
}
