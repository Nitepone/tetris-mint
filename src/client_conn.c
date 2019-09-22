#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>

#include "client_conn.h"

// for backward compatibility
#define h_addr h_addr_list[0]

// socket to communicate with server
static int sock_fd = -1;
struct sockaddr_in servername;

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
tetris_send_message (char * message)
{
  int nbytes = write (sock_fd, message, strlen (message) + 1);
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
tetris_translate(int x, int y)
{
  char message[128];
  sprintf(message, "TRANSLATE:%d,%d", x, y);
  tetris_send_message(message);
}

void
tetris_rotate(int theta)
{
  char message[128];
  sprintf(message, "ROTATE:%d", theta);
  tetris_send_message(message);
}

void
tetris_drop()
{
  tetris_send_message("DROP");
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

void
tetris_disconnect()
{
  close(sock_fd);
}

/**
 * start listening to the server for updates to the board
 *
 * run this as a seperate thread
 */
void
tetris_listen(int board[10][24])
{
  // TODO
  return;
}
