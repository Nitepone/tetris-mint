/*
 * tetris-game.c
 * Copyright (C) 2019-2022 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "os_compat.h"
#include "tetris_game.h"
#include "tetris_game_priv.h"

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
static int destroy_block(struct active_block **block) {
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
static int spawn_active_block(struct active_block **block,
                              struct tetris_block tetris_block) {
	// allocate and fill
	(*block)->tetris_block = tetris_block;
	(*block)->position = ((struct position)BLOCK_START_POSITION);
	(*block)->rotation = none;
	return 0;
}

static int generate_new_block(struct game_contents *game_contents) {
	int rng = rand_r(&(game_contents)->seed) % ARRAY_SIZE(available_blocks);
	spawn_active_block(&game_contents->active_block,
	                   game_contents->next_block);
	game_contents->next_block = available_blocks[rng];
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
	(*game_contents)->shadow_block = calloc(1, sizeof(struct active_block));
	// set values
	(*game_contents)->seed = seed;
	(*game_contents)->auto_lower_count = 0;
	(*game_contents)->swap_h_block_count = 0;
	(*game_contents)->hold_block = tetris_block_null;
	(*game_contents)->lines_cleared = 0;
	(*game_contents)->points = 0;
	(*game_contents)->next_block =
	    available_blocks[rand_r(&(*game_contents)->seed) %
	                     ARRAY_SIZE(available_blocks)];
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
 * Populates active_block->board_units based on block type offsets.
 */
static int get_block_positions(struct active_block *block) {
	int x, y, i;
	struct position cur_offset_pos;
	for (i = 0; i < block->tetris_block.cell_count; i++) {
		// get one offset
		cur_offset_pos = block->tetris_block.position_offsets[i];
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
 * Tests if a block can be placed in the current location
 */
static int test_block(struct game_contents *gc,
                      struct active_block *new_block) {
	int i;
	struct position cur_unit_pos;
	// catch NULL block
	if (!new_block)
		return -1;
	get_block_positions(new_block);
	// test each pos
	for (i = 0; i < gc->active_block->tetris_block.cell_count; i++) {
		cur_unit_pos = new_block->board_units[i];
		if (cur_unit_pos.x < 0 || cur_unit_pos.x >= BOARD_WIDTH ||
		    cur_unit_pos.y < 0 || cur_unit_pos.y >= BOARD_HEIGHT ||
		    gc->board[cur_unit_pos.y][cur_unit_pos.x]) {
			return -2;
		}
	}
	return 0;
}

static int clone_block(struct active_block *old_block,
                       struct active_block **new_block) {
	if (!old_block) // NULL catch
		return -1;
	*new_block = calloc(1, sizeof(*old_block));
	memcpy(*new_block, old_block, sizeof(*old_block));
	return 0;
}

static int delete_line(int line_number, struct game_contents *game_contents) {
	int j, i;
	for (j = (line_number + 1); j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			game_contents->board[j - 1][i] =
			    game_contents->board[j][i];
		}
	}
	return 0;
}

static int cull_lines(struct game_contents *game_contents) {
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
	// Iterate over all the tiles in the board that are considered
	// out-of-play, ie. the tiles that are above the playable area.
	for (j = BOARD_PLAY_HEIGHT; j < BOARD_HEIGHT; j++) {
		for (i = 0; i < BOARD_WIDTH; i++) {
			if (game_contents->board[j][i])
				return 1;
		}
	}
	return 0;
}

static int place_block(struct game_contents *gc) {
	int i;
	struct position cur_unit_pos;
	get_block_positions(gc->active_block);
	// merge block into board array
	for (i = 0; i < gc->active_block->tetris_block.cell_count; i++) {
		cur_unit_pos = gc->active_block->board_units[i];
		gc->board[cur_unit_pos.y][cur_unit_pos.x] =
		    ((int)gc->active_block->tetris_block.type);
	}
	// check for lines
	cull_lines(gc);
	// check for game over
	if (game_over(gc))
		return 2;
	generate_new_block(gc);
	gc->swap_h_block_count = 0;
	gc->auto_lower_count = 0;
	return 0;
}

static int translate_block_helper(struct game_contents *gc, int distance) {
	struct active_block *new_block = NULL;
	clone_block(gc->active_block, &new_block);
	// perform translation
	new_block->position.x += distance;
	// test if move was valid
	if (!test_block(gc, new_block)) {
		gc->active_block->position = new_block->position;
		destroy_block(&new_block);
		return 0;
	} else {
		destroy_block(&new_block);
		return -1;
	}
}

int translate_block_right(struct game_contents *gc) {
	return translate_block_helper(gc, -1);
}

int translate_block_left(struct game_contents *gc) {
	return translate_block_helper(gc, 1);
}

int rotate_block(struct game_contents *gc, int clockwise) {
	int d;
	int i;
	enum rotation start_rot = gc->active_block->rotation;
	enum rotation end_rot;
	const struct position *srs_test = NULL;
	const struct srs_movement_descriptor *srs_desc = NULL;
	const struct srs_movement_mode *srs_mode =
	    gc->active_block->tetris_block.srs_mode;
	struct active_block *new_block = NULL;
	struct active_block *temp_block_p = NULL;

	// no-op on no SRS kick mode
	if (srs_mode == NULL) {
		return 0;
	}
	// calculate end_rot
	if (clockwise) {
		d = 1;
	} else {
		d = (ROT_COUNT - 1); // effectively (-1) as it is remaindered
	}
	end_rot = (start_rot + d) % ROT_COUNT;
	// find SRS descriptor for current movement
	for (i = 0; i < srs_mode->descriptor_count; i++) {
		srs_desc = (srs_mode->descriptor_arr + i);
		if ((srs_desc->start_rot == start_rot) &&
		    (srs_desc->end_rot == end_rot)) {
			break;
		}
		srs_desc = NULL;
	}
	// error if no SRS descriptor for current movement
	if (srs_desc == NULL) {
		return -2;
	}

	// perform rotation
	clone_block(gc->active_block, &new_block);
	new_block->rotation = end_rot;
	// attempt SRS test positions until match
	for (i = 0; i < srs_desc->test_count; i++) {
		srs_test = srs_desc->test_arr + i;
		new_block->position = (struct position){
		    gc->active_block->position.x + srs_test->x,
		    gc->active_block->position.y + srs_test->y};
		if (!test_block(gc, new_block)) {
			temp_block_p = gc->active_block;
			gc->active_block = new_block;
			destroy_block(&temp_block_p);
			return 0;
		}
	}
	// we have exhasted options.. can not rotate!
	return -1;
}

static int lower_block_helper(struct game_contents *gc,
                              struct active_block *block) {
	struct active_block *new_block = NULL;
	clone_block(block, &new_block);
	// perform transform
	new_block->position.y--;
	// test if move was valid
	if (!test_block(gc, new_block)) {
		block->position = new_block->position;
		destroy_block(&new_block);
		return -1;
	} else {
		// destroy translated block
		destroy_block(&new_block);
		return 0;
	}
}

int lower_block(struct game_contents *game_contents, int forced) {
	int ret = 0;
	ret = lower_block_helper(game_contents, game_contents->active_block);
	if (ret) {
		return ret;
	}
	// handle placing block if needed
	if (!forced) {
		return place_block(game_contents);
	} else {
		if (game_contents->auto_lower_count++ >= MAX_AUTO_LOWER)
			return place_block(game_contents);
	}
	return -1;
}

int hard_drop(struct game_contents *gc) {
	while (lower_block_helper(gc, gc->active_block))
		;
	return place_block(gc);
}

int generate_shadow_block(struct game_contents *gc) {
	destroy_block(&(gc->shadow_block));
	clone_block(gc->active_block, &(gc->shadow_block));
	while (lower_block_helper(gc, gc->shadow_block))
		;
	return 0;
}

int generate_game_view_data(struct game_contents *gc,
                            struct game_view_data **gvd) {
	int i;
	struct position cur_unit_pos;
	// alloc new gvd
	if (!(*gvd)) {
		*gvd = calloc(1, sizeof(struct game_view_data));
	}
	memcpy((*gvd)->board, gc->board,
	       sizeof(int) * BOARD_WIDTH * BOARD_HEIGHT);

	generate_shadow_block(gc);
	get_block_positions(gc->active_block);
	get_block_positions(gc->shadow_block);
	// draw shadow_block to board
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = gc->shadow_block->board_units[i];
		(*gvd)->board[cur_unit_pos.y][cur_unit_pos.x] =
		    ((int)(enum block_type)shadow);
	}
	// draw active_block to board (separate to ensure overwrite of shadow)
	for (i = 0; i < MAX_BLOCK_UNITS; i++) {
		cur_unit_pos = gc->active_block->board_units[i];
		(*gvd)->board[cur_unit_pos.y][cur_unit_pos.x] =
		    ((int)gc->active_block->tetris_block.type);
	}
	// save scores into gvd
	(*gvd)->lines_cleared = gc->lines_cleared;
	(*gvd)->points = gc->points;
	// save next and hold blocks
	(*gvd)->hold_block = gc->hold_block.type;
	(*gvd)->next_block = gc->next_block.type;
	return 0;
}

int swap_hold_block(struct game_contents *gc) {
	struct tetris_block active_type;

	// catch too many swaps between block placement
	if (gc->swap_h_block_count >= MAX_SWAP_H)
		return -1;

	if (gc->hold_block.type == no_type) {
		gc->hold_block = gc->active_block->tetris_block;
		generate_new_block(gc);
	} else {
		active_type = gc->hold_block;
		gc->hold_block = gc->active_block->tetris_block;
		spawn_active_block(&gc->active_block, active_type);
	}

	gc->swap_h_block_count++;
	return 0;
};

int get_tetris_block_offsets(const struct position **offset,
                             enum block_type type) {
	int i;
	for (i = 0; i < ARRAY_SIZE(available_blocks); i++) {
		if (type == available_blocks[i].type) {
			*offset = available_blocks[i].position_offsets;
			return available_blocks[i].cell_count;
		}
	}
	return -1;
}
