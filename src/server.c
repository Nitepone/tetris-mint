#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "player.h"

#define PORT 5555
// message must hold 24 * 10 integers
#define MAXMSG 1024

void send_board (int fd);

/**
 * get and bind a socket or exit on failure
 */
int
make_socket (uint16_t port)
{
	int sock;
	struct sockaddr_in name;

	// get a socket
	sock = socket (PF_INET, SOCK_STREAM, 0);
	if (sock < 0)
	{
		perror ("socket");
		exit (EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	name.sin_addr.s_addr = htonl (INADDR_ANY);
	if (bind (sock, (struct sockaddr *) &name, sizeof (name)) < 0)
	{
		perror ("bind");
		exit (EXIT_FAILURE);
	}

	return sock;
}

int
read_from_client (int filedes)
{
	char buffer[MAXMSG];

	int nbytes = read (filedes, buffer, MAXMSG);

	// exit early if there was an error
	if (nbytes < 0) {
		perror ("read");
		exit (EXIT_FAILURE);
	}

	// exit early if we reached the end-of-file
	if (nbytes == 0)
		return -1;

	// data was successfully read into the buffer
	fprintf (stderr, "Received from client: `%s'\n", buffer);

	// send board back to client
	// message_client (filedes, "Hello client");
	send_board(filedes);
	return 0;
}

void
send_client_nbytes (int fd, char * message, int n)
{
  int bytes_written = write (fd, message, n);
  fprintf(stderr, "Sent %d bytes to client\n", bytes_written);

  if (bytes_written < 0) {
    perror ("write");
    exit (EXIT_FAILURE);
  }
}

void
message_client (int fd, char * message)
{
  int nbytes = write (fd, message, strlen (message) + 1);
  if (nbytes < 0) {
    perror ("write");
    exit (EXIT_FAILURE);
  }
}

void
send_board (int fd)
{
	struct st_player * player = get_player_from_fd(fd);
	if (player == 0) {
		fprintf(stderr, "Error: No player found for socket\n");
		return;
	}

	if (player->view == 0) {
		fprintf(stderr, "Error: No view exists for player.\n");
		return;
	}

	if (player->view->board == 0) {
		fprintf(stderr, "Error: No board exists for player.\n");
		return;
	}

	char message[MAXMSG] = "BOARD";
	memcpy(message + 5, player->view->board, 960);
	send_client_nbytes(fd, message, 965);
}

int
main (void)
{
	int sock;
	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size;

	/* Create the socket and set it up to accept connections. */
	sock = make_socket (PORT);
	if (listen (sock, 1) < 0)
		{
			perror ("listen");
			exit (EXIT_FAILURE);
		}

	/* Initialize the set of active sockets. */
	FD_ZERO (&active_fd_set);
	FD_SET (sock, &active_fd_set);

	/* Initialize the player list */
	player_init();

	while (1)
	{
		/* Block until input arrives on one or more active sockets. */
		read_fd_set = active_fd_set;
		if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
		{
			perror ("select");
			exit (EXIT_FAILURE);
		}

		/* Service all the sockets with input pending. */
		for (i = 0; i < FD_SETSIZE; ++i)
			if (FD_ISSET (i, &read_fd_set))
			{
				if (i == sock)
				{
					/* Connection request on original socket. */
					int new;
					size = sizeof (clientname);
					new = accept (sock,
						(struct sockaddr *) &clientname,
						(socklen_t *) &size);
					if (new < 0)
					{
						perror ("accept");
						exit (EXIT_FAILURE);
					}
					fprintf (stderr,
						 "Server: connect from host %s, port %hd.\n",
						 inet_ntoa (clientname.sin_addr),
						 ntohs (clientname.sin_port));

					/* add the new player */
					player_create(new, "George");
					// write(new, "hello world", 12 );
					// send_board(new);

					// write(new, "receiv\u2588", 9);
					FD_SET (new, &active_fd_set);
				}
				else
				{
					/* Data arriving on an already-connected socket. */
					if (read_from_client (i) < 0)
					{
						close (i);
						FD_CLR (i, &active_fd_set);
					}
				}
			}
	}
}

// vi:noet:noai:sw=0:sts=0:ts=8
