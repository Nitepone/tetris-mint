/*
 * tetris-game.h
 * Copyright (C) 2019-2022 nitepone <luna@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

#include <stddef.h>

#define BOARD_HEIGHT 24
#define BOARD_PLAY_HEIGHT 20
#define BOARD_WIDTH 10

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

enum rotation {
	none = 0,
	right = 1,
	invert = 2,
	left = 3,
};
#define ROT_COUNT 4

enum block_type {
	no_type,
	shadow,
	orange,
	blue,
	cleve,
	rhode,
	teewee,
	hero,
	smashboy,
};

struct game_contents;
struct active_block;
struct srs_movement_mode;

struct position {
	int x;
	int y;
};

struct tetris_block {
	enum block_type type;
	unsigned int cell_count;
	const struct position *position_offsets;
	const struct srs_movement_mode *srs_mode;
};

struct game_view_data {
	int points;
	int lines_cleared;
	enum block_type next_block;
	enum block_type hold_block;
	int board[BOARD_HEIGHT][BOARD_WIDTH];
};

/**
 * Lowers the block down the board by 1
 * @param forced - 0 if move is done by client, non-zero if by game
 * @return - 0 if game is still good, -1 if the block cannot be lowered
 */
int lower_block(struct game_contents *game_contents, int forced);

/*
 * Translates a block left a unit
 * @return - 0 if piece moved, else non-zero
 */
int translate_block_left(struct game_contents *gc);

/*
 * Translates a block right a unit
 * @return - 0 if piece moved, else non-zero
 */
int translate_block_right(struct game_contents *gc);

/*
 * Rotates a block left or right
 * @param clockwise - zero rotates the block counter-clockwise, else clockwise
 * @return - 0 if piece rotated, else non-zero
 */
int rotate_block(struct game_contents *game_contents, int clockwise);

/*
 * Starts a game and takes a double pointer for game contents data
 * @return 0
 */
int new_game(struct game_contents **game_contents);

/*
 * Starts a game and takes a double pointer for game contents data.
 * Also takes a seed to base the number generation from.
 * @return 0
 */
int new_seeded_game(struct game_contents **game_contents, unsigned int seed);

/*
 * Destroy old game data
 * Frees the memory and sets the second pointer to NULL
 * @return 0
 */
int destroy_game(struct game_contents **game_contents);

/*
 * Makes a game_view_data with a representation of all active board pieces
 */
int generate_game_view_data(struct game_contents *gc,
                            struct game_view_data **gvd);

/**
 * Check if the game is over
 * @param game_contents
 * @return 0 if game is still ongoing or 1 if the game is over
 */
int game_over(struct game_contents *game_contents);

/*
 * Forces the current piece to drop until placed immediately
 */
int hard_drop(struct game_contents *game_contents);

/*
 * Swaps the currently used block with the one in holding
 */
int swap_hold_block(struct game_contents *game_contents);

/*
 * Gets the offsets for a tetris block based on a block_type enum value.
 *
 * @param offset A double pointer to point to the correct position array
 * @param type the enum type of the desired block
 * @return The length of the offset array. Negative on failure.
 */
int get_tetris_block_offsets(const struct position **offset,
                             enum block_type type);

#endif /* !TETRIS_GAME_H */
