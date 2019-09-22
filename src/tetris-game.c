/*
 * tetris-game.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tetris-game.h"

int destroy_game (struct game_contents **game_contents) {
	struct game_contents *gc_temp;
	if (!(*game_contents)) // NULL catch
		return -1;
	// create a temp pointer and set game_contents NULL
	gc_temp = *game_contents;
	*game_contents = NULL;
	// free mem
	free(gc_temp->active_block);
	free(gc_temp);
	return 0;
}

int destroy_block (struct active_block **block) {
	struct active_block *ab_temp;
	if (!(*block)) // NULL catch
		return -1;
	ab_temp = *block;
	*block = NULL;
	free(ab_temp);
	return 0;
}

int generate_block (struct active_block **block) {
	// clear pointer
	destroy_block(block);
	// allocate and fill
	*block = calloc(1, sizeof(struct active_block));
	(*block)->block_type = rand() % (MAX_BLOCK_UNITS - 2); //TODO: Remove debug that prevents special bocks
	(*block)->position = ((struct position) { 4, 20 }); //TODO: Move to header
	(*block)->rotation_angle = ROT_NONE;
	return 0;
}

int new_game (struct game_contents **game_contents) {
	// destroy old memory cleanly
	destroy_game(game_contents);
	// allocate new memory
	*game_contents = calloc(1, sizeof(struct game_contents));
	(*game_contents)->active_block = calloc(1, sizeof(struct active_block));
	// set values
	generate_block(&(*game_contents)->active_block);
	return 0;
}

int get_def_block_positions (struct active_block *block) {
	int x, y, i;
	struct position cur_offset_pos;
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		// get one offset
		cur_offset_pos = block_offsets[i][block->block_type];
		// rotate
		switch (block->rotation_angle) {
			// TODO: Review the math here, we might be mirroring
			case ROT_NONE:
				x = cur_offset_pos.x;
				y = cur_offset_pos.y;
				break;
			case ROT_RIGHT:
				x = cur_offset_pos.y;
				y = cur_offset_pos.x;
				break;
			case ROT_INVERT:
				x = (-1) * cur_offset_pos.x;
				y = (-1) * cur_offset_pos.y;
				break;
			case ROT_LEFT:
				x = (-1) * cur_offset_pos.y;
				y = (-1) * cur_offset_pos.x;
				break;
		}
		// convert offset to position and save
		block->board_units[i] = ((struct position) {
			block->position.x + x,
			block->position.y + y
		});
	}
	return 0;
}

int get_block_positions (struct active_block *block) {
	switch (block->block_type) {
		// standard blocks
		case orange:
		case blue:
		case cleve:
		case rhode:
		case teewee:
			return get_def_block_positions(block);
		// special blocks
		case smashboy:
		case hero:
			return -1;
	}
	return 0;
}

int test_block (struct game_contents *game_contents,
		struct active_block *new_block) {
	int i;
	struct position cur_unit_pos;
	// catch NULL block
	if (!new_block)
		return -1;
	get_block_positions(new_block);
	// test each pos
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = new_block->board_units[i];
		if ( cur_unit_pos.x < 0 || cur_unit_pos.x >= 10 ||
		     cur_unit_pos.y < 0 || cur_unit_pos.y >= 24 ||
		     game_contents->board[cur_unit_pos.y][cur_unit_pos.x]
		) {
			return -2;
		}
	}
	return 0;
}

int clone_block (struct active_block *old_block,
		 struct active_block **new_block) {
	if (!old_block) // NULL catch
		return -1;
	destroy_block(new_block);
	*new_block = calloc(1, sizeof(struct active_block));
	**new_block = *old_block;
	return 0;
}

int rotate_block (int clockwise, struct game_contents *game_contents) {
	int x;
	struct active_block *new_block = NULL;
	clone_block(game_contents->active_block, &new_block);
	if (clockwise)
		x = 1;
	else
		x = (-1);
	// perform rotation
	new_block->rotation_angle = (new_block->rotation_angle + x) % ROT_COUNT;
	// test if move was valid
	if (!test_block(game_contents, new_block)) {
		//printf("Block moved down: %d %d\n",
		//		new_block->position.y);
		game_contents->active_block = new_block;
	} else {
		printf("Block can not rotate\n");
		return -1;
	}
	return 0;
}

int lower_block (int forced, struct game_contents *game_contents) {
	struct active_block *new_block = NULL;
	clone_block(game_contents->active_block, &new_block);
	// perform transform
	new_block->position.y--;
	// test if move was valid
	if (!test_block(game_contents, new_block)) {
		//printf("Block moved down: %d %d\n",
		//		new_block->position.y);
		game_contents->active_block = new_block;
	} else {
		printf("Block can not move further\n");
		return -1;
	}
	return 0;
}
