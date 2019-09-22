/*
 * test-main.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>

#include "tetris-game.h"

struct game_contents *game_contents;

int main() {
	printf("Hello\n");
	new_game(&game_contents);
	printf("Created Game: %d %d\n", game_contents->board[0][0], game_contents->active_block);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	lower_block(1, game_contents);
	return 0;
}
