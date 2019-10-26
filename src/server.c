#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <asm/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>

#include "player.h"

// message must hold 24 * 10 integers
#define MAXMSG 1024

#define MSG_TYPE_REGISTER 'U'
#define MSG_TYPE_ROTATE 'R'
#define MSG_TYPE_TRANSLATE 'T'
#define MSG_TYPE_LOWER 'L'

void send_board (struct st_player * player);

/**
 * get and bind a socket or exit on failure
 */
int
make_socket (char * host, uint16_t port)
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

	// forcefully attaching socket to the port
	int opt = 1;
	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
			&opt, sizeof(opt))) {
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}

	/* Give the socket a name. */
	name.sin_family = AF_INET;
	name.sin_port = htons (port);
	inet_pton(AF_INET, host, &name.sin_addr.s_addr);
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

	// handle input
	Player * player = get_player_from_fd(filedes);
	char name[16];
	switch(buffer[0]){
	case MSG_TYPE_REGISTER:
		sscanf( buffer + 1, "%15s", name);
		player = player_create(filedes, name);
		player->render = send_board;
		printf("Registered new user: %s\n", name);
		break;
	case MSG_TYPE_ROTATE:
		rotate_block(buffer[1], player->contents);
		break;
	case MSG_TYPE_TRANSLATE:
		translate_block(buffer[1], player->contents);
		break;
	case MSG_TYPE_LOWER:
		lower_block(0, player->contents);
		break;
	}
	send_board(player);

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
send_board (struct st_player * player)
{
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
	generate_game_view_data(&player->view, player->contents);
	memcpy(message + 5, player->view->board, 960);
	send_client_nbytes(player->fd, message, 965);
}

void
usage()
{
  fprintf(stderr, "Usage: ./server ADDRESS PORT\n");
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

	int sock;
	fd_set active_fd_set, read_fd_set;
	int i;
	struct sockaddr_in clientname;
	size_t size;

	/* Create the socket and set it up to accept connections. */
	sock = make_socket (host, numeric_port);
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
		{
			// exit early if the file descriptor i is not in the set
			if (!FD_ISSET (i, &read_fd_set))
				continue;

			// for new connections:
			// - accept the connection
			// - create a file descriptor for the connection
			// - add the file descriptor to the file descriptor set
			if (i == sock)
			{
				size = sizeof (clientname);
				int new = accept (sock,
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

				FD_SET (new, &active_fd_set);
			}
			// handle data on sockets already in the file descriptor set
			else if (read_from_client (i) < 0) {
				close (i);
				FD_CLR (i, &active_fd_set);
			}
		}
	}
}

// vi:noet:noai:sw=0:sts=0:ts=8
