#include <stdlib.h>

#include "controller.h"
#include "player.h"
#include "render.h"
#include "tetris_game.h"

/**
 * player struct for this offline game
 */
static Player *offline_player;

/**
 * Render function that draws the board using NCurses for a given player. The
 * fd argument exists for compatibility only (with the online code) and is not
 * used. After refactoring, it will likely be removed.
 */
static int renderish(int fd, Player *player) {
	generate_game_view_data(&player->view, player->contents);
	render_game_view_data(player->name, player->view);
	return EXIT_SUCCESS;
}

static void offline_translate(int x) {
	translate_block(x, offline_player->contents);
	renderish(0, offline_player);
}

static void offline_lower() {
	lower_block(0, offline_player->contents);
	renderish(0, offline_player);
}

static void offline_rotate(int theta) {
	rotate_block(theta, offline_player->contents);
	renderish(0, offline_player);
}

static void offline_drop() {
	hard_drop(offline_player->contents);
	renderish(0, offline_player);
}

// define a control set for use offline
static const TetrisControlSet OfflineControlSet = {.translate =
                                                       offline_translate,
                                                   .lower = offline_lower,
                                                   .rotate = offline_rotate,
                                                   .drop = offline_drop};

/**
 * creates an offline control set for the given player and set the render
 * function for the player
 */
TetrisControlSet offline_control_set(Player *player) {
	offline_player = player;
	offline_player->render = renderish;
	return OfflineControlSet;
}
