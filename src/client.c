#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_conn.h"
#include "offline.h"
#include "player.h"
#include "render.h"
#include "tetris_game.h"
#include "widgets.h"

/**
 * print the usage and exit
 */
void usage() {
	fprintf(stderr,
	        "Usage: ./client [-h] [-l] [-s] [-a ADDRESS] [-p PORT]\n");
	exit(EXIT_FAILURE);
}

/**
 * run an offline game of tetris
 */
int run_offline() {
	// initialize the player list
	player_init();

	// create a single player
	char *names[1];
	names[0] = "You";
	Player *player = player_create(0, names[0]);
	start_game(player);

	render_init(1, names);
	keyboard_input_loop(offline_control_set(player), NULL);
	render_close();
	return EXIT_SUCCESS;
}

/**
 * list the online players and then exit
 */
int run_list_online_players(char *host, int port) {
	NetClient *net_client = net_client_init();
	tetris_connect(net_client, host, port);

	tetris_listen(net_client);

	StringArray *names = tetris_list(net_client);
	for (int i = 0; i < names->length; i++) {
		fprintf(stderr, "%s\n", string_array_get_item(names, i));
	}

	tetris_disconnect(net_client);
	return EXIT_SUCCESS;
}

/**
 * run an online game of tetris
 */
int run_online(char *host, int port, char *username, char *opponent) {
	// initialize the player list
	player_init();

	// initialize the renderer
	// THIS MUST BE DONE BEFORE REGISTERING WITH THE SERVER
	char *names[2];
	names[0] = username;
	names[1] = opponent;
	render_init(2, names);

	// connect to the server
	NetClient *net_client = net_client_init(host, port);
	tetris_connect(net_client, host, port);

	// create our player
	Player *player = player_create(0, username);
	player->fd = net_client->fd;
	net_client->player = player;

	// register our player
	tetris_register(net_client, username);
	tetris_opponent(net_client, opponent);

	tetris_listen(net_client);

	// create the renderer and start the input loop
	keyboard_input_loop(tcp_control_set(), net_client);

	render_close();

	tetris_disconnect(net_client);
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	char username[32] = "Anonymous";
	char host[128] = "127.0.0.1";
	char port[6] = "5555";

	int should_run_offline = 0;
	int list_players = 0;

	// Parse command line flags. The optstring passed to getopt has a
	// preceding colon to tell getopt that missing flag values should be
	// treated differently than unknown flags. The proceding colons indicate
	// that flags must have a value.
	int opt;
	while ((opt = getopt(argc, argv, ":hsla:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 's':
			should_run_offline = 1;
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

	if (list_players)
		return run_list_online_players(host, numeric_port);

	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);

	char *choices[] = {
	    "Single",
	    "Multi",
	    "Exit",
	    (char *)NULL,
	};
	int result = ttviz_select(choices, 4, "Gameplay Mode");

	// exit chosen
	if (result == 2) {
		printf("Goodbye!\n");
		exit(0);
	}

	// single player mode chosen
	if (should_run_offline || result == 0) {
		return run_offline();
	}

	ttviz_entry(username, "Enter username: ");

	// get the list of opponents
	NetClient *net_client = net_client_init();
	tetris_connect(net_client, host, numeric_port);
	tetris_listen(net_client);
	StringArray *names = tetris_list(net_client);
	tetris_disconnect(net_client);

	char *opponent = "";
	if (names->length > 0) {
		// null terminate the string array
		string_array_resize(names, names->length + 1);

		int opponent_index =
		    ttviz_select(names->strings, names->length, "Opponent");
		opponent = string_array_get_item(names, opponent_index);
	}

	return run_online(host, numeric_port, username, opponent);
}
