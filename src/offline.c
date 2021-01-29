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

static void offline_translate(void *context, int x) {
	if (x) {
		translate_block_left(offline_player->contents);
	} else {
		translate_block_right(offline_player->contents);
	}
	renderish(0, offline_player);
}

static void offline_lower(void *context) {
	lower_block(0, offline_player->contents);
	renderish(0, offline_player);
}

static void offline_rotate(void *context, int theta) {
	rotate_block(theta, offline_player->contents);
	renderish(0, offline_player);
}

static void offline_drop(void *context) {
	hard_drop(offline_player->contents);
	renderish(0, offline_player);
}

static void offline_swap_hold(void *context) {
	swap_hold_block(offline_player->contents);
	renderish(0, offline_player);
}

// define a control set for use offline
static const TetrisControlSet OfflineControlSet = {
    .translate = offline_translate,
    .lower = offline_lower,
    .rotate = offline_rotate,
    .drop = offline_drop,
    .swap_hold = offline_swap_hold};

/**
 * creates an offline control set for the given player and set the render
 * function for the player
 */
TetrisControlSet offline_control_set(Player *player) {
	offline_player = player;
	offline_player->render = renderish;
	return OfflineControlSet;
}
