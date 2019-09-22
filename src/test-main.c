/*
 * test-main.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>

#include "tetris-game.h"

struct game_contents *game_contents;
struct game_view_data *gvd;

void print_board() {
	int i, j;
	generate_game_view_data(&gvd, game_contents);
	for (i = 23; i >= 0; i--) {
		for (j = 9; j >= 0; j--) {
			printf("%d", gvd->board[i][j]);
		}
		printf("\n");
	}
	printf("Points: %d   Lines: %d", game_contents->points, game_contents->lines_cleared);
	printf("\n");
}

int main() {
	int i;

	new_game(&game_contents);
	printf("Created Game: %d %d\n", game_contents->board[0][0], game_contents->active_block);
	print_board();

	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	lower_block(0, game_contents);
	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	lower_block(0, game_contents);

	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	for (i = 0; i < 20; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	translate_block(1, game_contents);
	translate_block(1, game_contents);
	for (i = 0; i < 20; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	/*
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	*/
	rotate_block(0, game_contents);

	for (i = 0; i < 22; i++)
		lower_block(1, game_contents);

	//rotate_block(1, game_contents);
	print_board();
	return 0;
}
