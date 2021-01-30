#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_conn.h"
#include "event.h"
#include "log.h"
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
	player_game_start(player);

	render_init(1, names);
	keyboard_input_loop(offline_control_set(player), NULL);

	player_game_stop(player);
	render_close();
	return EXIT_SUCCESS;
}

/**
 * list the online players and then exit
 */
int run_list_online_players(char *host, int port) {
	NetClient *net_client = net_client_init();
	if (tetris_connect(net_client, host, port) == EXIT_FAILURE) {
		perror("run_list_online_players");
		return EXIT_FAILURE;
	}

	tetris_listen(net_client);

	StringArray *names = tetris_list(net_client);
	for (int i = 0; i < names->length; i++) {
		fprintf(stderr, "%s\n", string_array_get_item(names, i));
	}

	tetris_disconnect(net_client);
	return EXIT_SUCCESS;
}

void *player_lobby(void *_net_client) {
	NetClient *net_client = (NetClient *)_net_client;
	// get the list of possible opponents
	StringArray *online_usernames = tetris_list(net_client);
	while (online_usernames->length == 0) {
		sleep(2);
		online_usernames = tetris_list(net_client);
	};

	WidgetSelection *selection = ttviz_select(
	    online_usernames->strings, online_usernames->length, "Opponent", 0);
	StringArray *opponents = selection_to_string_array(selection);
	tetris_opponent(net_client, opponents);

	tetris_tell_server_to_start(net_client);

	// free up resources
	string_array_destroy(online_usernames);
	selection_destroy(selection);

	return NULL;
}

/**
 * run an online game of tetris
 */
int run_online(char *host, int port) {
	// initialize the player list
	player_init();

	// connect to the server
	NetClient *net_client = net_client_init(host, port);
	if (tetris_connect(net_client, host, port) == EXIT_FAILURE) {
		// print the error at the top-left of the terminal using curses
		mvprintw(0, 0, strerror(errno));
		return EXIT_FAILURE;
	}
	tetris_listen(net_client);

	// prompt for a username
	char username[32];
	ttviz_entry(username, "Enter username: ", PLAYER_NAME_MAX_CHARS);

	// create our player
	Player *player = player_create(0, username);
	player->fd = net_client->fd;
	net_client->player = player;

	// register our player
	NetRequest *request = tetris_register(net_client, username);
	ttetris_net_request_block_for_response(request);

	fprintf(
	    stderr,
	    "Registered successfully! Fetching online players from server...");

	// get the list of possible opponents

	pthread_t _thread;
	pthread_create(&_thread, NULL, player_lobby, net_client);

	// block until we hear from the server that the game has started
	ttetris_event_block_for_completion(player->game_start_event);

	// cancel the lobby thread if it is still running
	pthread_cancel(_thread);

	// create the renderer and start the input loop
	keyboard_input_loop(tcp_control_set(), net_client);

	render_close();

	tetris_disconnect(net_client);
	return EXIT_SUCCESS;
}

/**
 * Show the main menu for gameplay selection
 */
int main_menu(char *host, int port) {
	WidgetSelection *selection;
	int selected_choice;
	char *choices[] = {
	    "Single", "Multi", "Controls", "Exit", (char *)NULL,
	};

	while (1) {
		// TODO We shouldn't need to be calling all this setup
		// everytime. The main change is that the init_pair calls are
		// different for the gameplay vs. the menu
		initscr();
		start_color();
		cbreak();
		noecho();
		keypad(stdscr, TRUE);
		init_pair(1, COLOR_RED, COLOR_BLACK);
		selection = ttviz_select(choices, 4, "Gameplay Mode", 1);
		selected_choice = selection_to_index(selection);

		switch (selected_choice) {
		case 0:
			run_offline();
			break;
		case 1:
			run_online(host, port);
			break;
		case 2:
			// TODO Implement the keybindings / controls menu (#21)
			break;
		case 3:
			return EXIT_SUCCESS;
		default:
			perror("Unexpected state");
			return EXIT_FAILURE;
		}

		// TODO Cleanup
	}
}

int main(int argc, char *argv[]) {
	char host[128] = "127.0.0.1";
	char port[6] = "5555";

	int list_players = 0;

	// set the logger file pointer to /dev/null
	logging_set_fp(fopen("/dev/null", "w"));

	// Parse command line flags. The optstring passed to getopt has a
	// preceding colon to tell getopt that missing flag values should be
	// treated differently than unknown flags. The proceding colons indicate
	// that flags must have a value.
	int opt;
	while ((opt = getopt(argc, argv, ":hla:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
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

	int exit_code = main_menu(host, numeric_port);

	// make sure terminal is functional when we exit
	endwin();

	return exit_code;
}
