#include <errno.h>
#include <inttypes.h>
#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "client_conn.h"
#include "controller.h"
#include "player.h"
#include "render.h"

Player *offline_player;
struct game_contents *offline_contents;

// local cache of the user's tetris board
// global variables and static variables are automatically initialized to zero
int board[BOARD_HEIGHT][BOARD_WIDTH];

void usage() {
	fprintf(stderr, "Usage: ./client [-h] [-u USERNAME] [-o OPPONENT] [-a "
	                "ADDRESS] [-p PORT]\n");
	exit(EXIT_FAILURE);
}

static int renderish(int fd, Player *player) {
	generate_game_view_data(&player->view, player->contents);
	render_board(player->name, player->view->board);
	return EXIT_SUCCESS;
}

static void offline_translate(int x) {
	translate_block(x, offline_contents);
	renderish(0, offline_player);
}

static void offline_lower() {
	lower_block(0, offline_contents);
	renderish(0, offline_player);
}

static void offline_rotate(int theta) {
	rotate_block(theta, offline_contents);
	renderish(0, offline_player);
}

static void offline_drop() {
	hard_drop(offline_contents);
	renderish(0, offline_player);
}

// define a control set for use offline
static const TetrisControlSet OfflineControlSet = {.translate =
                                                       offline_translate,
                                                   .lower = offline_lower,
                                                   .rotate = offline_rotate,
                                                   .drop = offline_drop};

int solo() {
	// initialize the player list
	player_init();

	// create a single player
	char *names[1];
	names[0] = "You";
	Player *player = player_create(0, names[0]);
	player->render = renderish;

	offline_contents = player->contents;
	offline_player = player;

	render_init(1, names);
	// render_board(player->name, board);
	// render_close();

	// return EXIT_SUCCESS;
	keyboard_input_loop(OfflineControlSet);
	render_close();
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	char username[32] = "Elliot";
	char opponent[32] = "";
	char host[128] = "127.0.0.1";
	char port[6] = "5555";

	int list_players = 0;

	// Parse command line flags. The optstring passed to getopt has a
	// preceding colon to tell getopt that missing flag values should be
	// treated differently than unknown flags. The proceding colons indicate
	// that flags must have a value.
	int opt;
	while ((opt = getopt(argc, argv, ":hslu:o:a:p:")) != -1) {
		switch (opt) {
		case 'h':
			usage();
			break;
		case 's':
			return solo();
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
	tetris_register(username);
	tetris_opponent(opponent);

	char *names[2];
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
		keyboard_input_loop(tcp_control_set());
		render_close();
	}

	tetris_disconnect();
	return 0;
}
