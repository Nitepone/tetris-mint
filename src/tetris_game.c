/*
 * tetris-game.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "tetris_game.h"

/*
 * Destroys a game_contents and frees memory
 */
int destroy_game(struct game_contents **game_contents) {
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

/*
 * Destroys an active_block and frees memory
 */
int destroy_block(struct active_block **block) {
	struct active_block *ab_temp;
	if (!(*block)) // NULL catch
		return -1;
	ab_temp = *block;
	*block = NULL;
	free(ab_temp);
	return 0;
}

/*
 * Generates a block at the top of the game board
 */
int spawn_active_block(struct active_block **block, enum block_type type) {
	// allocate and fill
	(*block)->block_type = type;
	(*block)->position = ((struct position)BLOCK_START_POSITION);
	(*block)->rotation = none;
	return 0;
}

int generate_new_block(struct game_contents *game_contents) {
	spawn_active_block(&game_contents->active_block,
	                   game_contents->next_block);
	game_contents->next_block =
	    rand_r(&game_contents->seed) % BLOCK_TYPE_COUNT;
	return 0;
}

/*
 * Initializes a game_contents struct in memory
 */
int new_seeded_game(struct game_contents **game_contents, unsigned int seed) {
	// destroy old memory cleanly
	destroy_game(game_contents);
	// allocate new memory
	*game_contents = calloc(1, sizeof(**game_contents));
	(*game_contents)->active_block = calloc(1, sizeof(struct active_block));
	// set values
	(*game_contents)->seed = seed;
	(*game_contents)->auto_lower_count = 0;
	(*game_contents)->swap_h_block_count = 0;
	(*game_contents)->hold_block = NO_BLOCK_VAL;
	(*game_contents)->lines_cleared = 0;
	(*game_contents)->points = 0;
	(*game_contents)->next_block =
	    rand_r(&(*game_contents)->seed) % BLOCK_TYPE_COUNT;
	generate_new_block(*game_contents);
	return 0;
}

/*
 * Start a game using a randomly generated seed
 */
int new_game(struct game_contents **game_contents) {
	return new_seeded_game(game_contents, time(NULL));
}

/*
 * Gets edge centric block positions
 * For use with blocks like the square block or line piece
 *
 * This rotates differently due to the "+ 1" when calculating the offsets
 */
int get_ec_block_pos(struct active_block *block) {
	int x, y, i;
	struct position cur_offset_pos;
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		// get one offset
		cur_offset_pos = block_offsets[block->block_type][i];
		// rotate
		switch (block->rotation) {
		case none:
			x = cur_offset_pos.x;
			y = cur_offset_pos.y;
			break;
		case right:
			x = cur_offset_pos.y;
			y = (-1) * cur_offset_pos.x + 1;
			break;
		case invert:
			x = (-1) * cur_offset_pos.x + 1;
			y = (-1) * cur_offset_pos.y + 1;
			break;
		case left:
			x = (-1) * cur_offset_pos.y + 1;
			y = cur_offset_pos.x;
			break;
		}
		// convert offset to position and save
		block->board_units[i] = ((struct position){
		    block->position.x + x, block->position.y + y});
	}
	return 0;
}

/*
 * Gets "block centric" block positions based on offsets and rotation
 */
int get_bc_block_pos(struct active_block *block) {
	int x, y, i;
	struct position cur_offset_pos;
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		// get one offset
		cur_offset_pos = block_offsets[block->block_type][i];
		// rotate
		switch (block->rotation) {
		case none:
			x = cur_offset_pos.x;
			y = cur_offset_pos.y;
			break;
		case right:
			x = cur_offset_pos.y;
			y = (-1) * cur_offset_pos.x;
			break;
		case invert:
			x = (-1) * cur_offset_pos.x;
			y = (-1) * cur_offset_pos.y;
			break;
		case left:
			x = (-1) * cur_offset_pos.y;
			y = cur_offset_pos.x;
			break;
		}
		// convert offset to position and save
		block->board_units[i] = ((struct position){
		    block->position.x + x, block->position.y + y});
	}
	return 0;
}

/*
 * This should direct blocks that are drawn using unit relational drawing
 * and special blocks to their associated functions.
 */
int get_block_positions(struct active_block *block) {
	switch (block->block_type) {
	// standard blocks
	case orange:
	case blue:
	case cleve:
	case rhode:
	case teewee:
		return get_bc_block_pos(block);
	// special blocks
	case smashboy:
	case hero:
		return get_ec_block_pos(block);
	}
	return 0;
}

/*
 * Tests if a block can be placed in the current location
 */
int test_block(struct game_contents *game_contents,
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
		if (cur_unit_pos.x < 0 || cur_unit_pos.x >= BOARD_WIDTH ||
		    cur_unit_pos.y < 0 || cur_unit_pos.y >= BOARD_HEIGHT ||
		    game_contents->board[cur_unit_pos.y][cur_unit_pos.x]) {
			return -2;
		}
	}
	return 0;
}

int clone_block(struct active_block *old_block,
                struct active_block **new_block) {
	if (!old_block) // NULL catch
		return -1;
	*new_block = calloc(1, sizeof(*old_block));
	memcpy(*new_block, old_block, sizeof(*old_block));
	return 0;
}

int delete_line(int line_number, struct game_contents *game_contents) {
	int j, i;
	for (j = (line_number + 1); j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			game_contents->board[j - 1][i] =
			    game_contents->board[j][i];
		}
	}
	return 0;
}

int cull_lines(struct game_contents *game_contents) {
	int i, j, units_in_row;
	int lines_culled = 0;
	// iterate and count each row/line
	for (j = 0; j < BOARD_HEIGHT; j++) {
		units_in_row = 0;
		for (i = 0; i < BOARD_WIDTH; i++) {
			if (game_contents->board[j][i])
				units_in_row++;
		}
		if (units_in_row >= BOARD_WIDTH) {
			delete_line(j, game_contents);
			j--;
			lines_culled++;
		}
	}
	// update scores
	game_contents->lines_cleared += lines_culled;
	switch (lines_culled) {
	case 0:
		break;
	case 1:
		game_contents->points += 100;
		break;
	case 2:
		game_contents->points += 250;
		break;
	case 3:
		game_contents->points += 500;
		break;
	case 4:
		game_contents->points += 1000;
		break;
	default:
		game_contents->points += 777;
		break;
	}

	return 0;
}

int game_over(struct game_contents *game_contents) {
	int i, j;
	// iterate and count each row/line
	for (j = BOARD_PLAY_HEIGHT; j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			if (game_contents->board[j][i])
				return 1;
		}
	}
	return 0;
}

int place_block(struct game_contents *game_contents) {
	int i;
	struct position cur_unit_pos;
	get_block_positions(game_contents->active_block);
	// merge block into board array
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = game_contents->active_block->board_units[i];
		game_contents->board[cur_unit_pos.y][cur_unit_pos.x] =
		    ((int)game_contents->active_block->block_type + 1);
	}
	// check for lines
	cull_lines(game_contents);
	// check for game over
	if (game_over(game_contents))
		return 2;
	generate_new_block(game_contents);
	game_contents->swap_h_block_count = 0;
	return 0;
}

int translate_block(int rightward, struct game_contents *game_contents) {
	struct active_block *new_block = NULL;
	struct active_block *temp_block_p = NULL;
	clone_block(game_contents->active_block, &new_block);
	// perform translation
	if (rightward)
		new_block->position.x++;
	else
		new_block->position.x--;
	// test if move was valid
	if (!test_block(game_contents, new_block)) {
		temp_block_p = game_contents->active_block;
		game_contents->active_block = new_block;
		destroy_block(&temp_block_p);
		return 0;
	} else {
		destroy_block(&new_block);
		return -1;
	}
}

int rotate_block(int clockwise, struct game_contents *game_contents) {
	int x;
	struct active_block *new_block = NULL;
	struct active_block *temp_block_p = NULL;
	clone_block(game_contents->active_block, &new_block);
	if (clockwise)
		x = 1;
	else
		x = (ROT_COUNT - 1); // effectively (-1) as it is remaindered
	// perform rotation
	new_block->rotation = (new_block->rotation + x) % ROT_COUNT;
	// test if move was valid
	if (!test_block(game_contents, new_block)) {
		temp_block_p = game_contents->active_block;
		game_contents->active_block = new_block;
		destroy_block(&temp_block_p);
		return 0;
	} else {
		destroy_block(&new_block);
		return -1;
	}
}

int lower_block(int auto_drop, struct game_contents *game_contents) {
	struct active_block *new_block = NULL;
	struct active_block *temp_block_p = NULL;
	clone_block(game_contents->active_block, &new_block);
	// perform transform
	new_block->position.y--;
	// test if move was valid
	if (!test_block(game_contents, new_block)) {
		temp_block_p = game_contents->active_block;
		game_contents->active_block = new_block;
		destroy_block(&temp_block_p);
		return -1;
	}

	// destroy translated block
	destroy_block(&new_block);
	// handle placing block if needed
	if (!auto_drop) {
		return place_block(game_contents);
	} else {
		if (game_contents->auto_lower_count++ >= MAX_AUTO_LOWER)
			return place_block(game_contents);
	}
	return -1;
}

int hard_drop(struct game_contents *gc) {
	int ret = 0;
	while (((ret = lower_block(0, gc)) < 0))
		;
	return ret;
}

int generate_game_view_data(struct game_view_data **gvd,
                            struct game_contents *gc) {
	int i;
	struct position cur_unit_pos;
	// alloc new gvd
	if (!(*gvd)) {
		*gvd = calloc(1, sizeof(struct game_view_data));
	}
	memcpy((*gvd)->board, gc->board,
	       sizeof(int) * BOARD_WIDTH * BOARD_HEIGHT);

	// draw current piece on gvd
	get_block_positions(gc->active_block);
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = gc->active_block->board_units[i];
		(*gvd)->board[cur_unit_pos.y][cur_unit_pos.x] =
		    ((int)gc->active_block->block_type) + 1;
	}
	// save scores into gvd
	(*gvd)->lines_cleared = gc->lines_cleared;
	(*gvd)->points = gc->points;
	// save next and hold blocks
	(*gvd)->hold_block = gc->hold_block;
	(*gvd)->next_block = gc->next_block;

	return 0;
}

int swap_hold_block(struct game_contents *gc) {
	enum block_type active_type;

	// catch too many swaps between block placement
	if (gc->swap_h_block_count >= MAX_SWAP_H)
		return -1;

	if (gc->hold_block == NO_BLOCK_VAL) {
		gc->hold_block = gc->active_block->block_type;
		generate_new_block(gc);
	} else {
		active_type = gc->hold_block;
		gc->hold_block = gc->active_block->block_type;
		spawn_active_block(&gc->active_block, active_type);
	}

	gc->swap_h_block_count++;
	return 0;
};
