/*
 * tetris-game.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tetris_game.h"

/*
 * Destroys a game_contents and frees memory
 */
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

/*
 * Destroys an active_block and frees memory
 */
int destroy_block (struct active_block **block) {
	struct active_block *ab_temp;
	if (!(*block)) // NULL catch
		return -1;
	ab_temp = *block;
	*block = NULL;
	free(ab_temp);
	return 0;
}

/*
 * Generates a random block
 */
int generate_block (struct active_block **block) {
	// allocate and fill
	(*block)->block_type = rand() % (BLOCK_TYPE_COUNT);
	(*block)->position = ((struct position) BLOCK_START_POSITION);
	(*block)->rotation = none;
	return 0;
}

/*
 * Initializes a game_contents struct in memory
 */
int new_game (struct game_contents **game_contents) {
	// destroy old memory cleanly
	destroy_game(game_contents);
	// allocate new memory
	*game_contents = calloc(1, sizeof(struct game_contents));
	(*game_contents)->active_block = calloc(1, sizeof(struct active_block));
	// set values
	(*game_contents)->lines_cleared = 0;
	(*game_contents)->points = 0;
	generate_block(&(*game_contents)->active_block);
	return 0;
}

/*
 * Gets "block centric" block positions based on offsets and rotation
 * ( Does not work proper for line and square as the center is not a single
 *   block. It will work though, just violates pure Tetris rules )
 */
int get_bc_block_pos (struct active_block *block) {
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
		block->board_units[i] = ((struct position) {
			block->position.x + x,
			block->position.y + y
		});
	}
	return 0;
}

/*
 * This should direct blocks that are drawn using unit relational drawing
 * and special blocks to their associated functions.
 */
int get_block_positions (struct active_block *block) {
	switch (block->block_type) {
		// standard blocks
		case orange:
		case blue:
		case cleve:
		case rhode:
		case teewee:
		// special blocks
		// TODO: Handle these correctly
		case smashboy:
		case hero:
			return get_bc_block_pos(block);
	}
	return 0;
}

/*
 * Tests if a block can be placed in the current location
 */
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
		if ( cur_unit_pos.x < 0 || cur_unit_pos.x >= BOARD_WIDTH ||
		     cur_unit_pos.y < 0 || cur_unit_pos.y >= BOARD_HEIGHT ||
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
	*new_block = calloc(1, sizeof(*old_block));
	memcpy(*new_block, old_block, sizeof(*old_block));
	return 0;
}

int delete_line (int line_number, struct game_contents *game_contents) {
	int j, i;
	for (j = (line_number + 1); j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			game_contents->board[j-1][i] =
				game_contents->board[j][i];
		}
	}
	return 0;
}

int cull_lines (struct game_contents *game_contents) {
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

int place_block (struct game_contents *game_contents) {
	int i;
	struct position cur_unit_pos;
	get_block_positions(game_contents->active_block);
	// merge block into board array
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = game_contents->active_block->board_units[i];
		game_contents->board[cur_unit_pos.y][cur_unit_pos.x] =
			((int) game_contents->active_block->block_type + 1);
	}
	// check for lines
	cull_lines (game_contents);
	// check for game over
	if (game_over(game_contents))
		return 2;
	generate_block(&game_contents->active_block);
	return 0;
}

int translate_block (int rightward, struct game_contents *game_contents) {
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

int rotate_block (int clockwise, struct game_contents *game_contents) {
	int x;
	struct active_block *new_block = NULL;
	struct active_block *temp_block_p = NULL;
	clone_block(game_contents->active_block, &new_block);
	if (clockwise)
		x = 1;
	else
		x = (ROT_COUNT-1); // effectively (-1) as it is remaindered
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

int lower_block (int auto_drop, struct game_contents *game_contents) {
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

int hard_drop (struct game_contents *gc) {
	int ret = 0;
	while (((ret = lower_block(0, gc)) < 0));
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
	memcpy((*gvd)->board, gc->board, sizeof(int) * 24 * 10);
	// draw current piece on gvd
	get_block_positions(gc->active_block);
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = gc->active_block->board_units[i];
		(*gvd)->board[cur_unit_pos.y][cur_unit_pos.x] =
			((int) gc->active_block->block_type ) + 1;
	}
	// saves scores into gvd
	(*gvd)->lines_cleared = gc->lines_cleared;
	(*gvd)->points = gc->points;

	return 0;
}
