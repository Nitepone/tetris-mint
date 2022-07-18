/*
 * tetris_game_priv.h
 *
 * Copyright (C) 2021-2022 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TETRIS_GAME_PRIV_H
#define TETRIS_GAME_PRIV_H

#include "tetris_game.h"

#define MAX_BLOCK_UNITS 4
#define BLOCK_START_POSITION                                                   \
	{ 4, 21 }

#define MAX_AUTO_LOWER 3
#define MAX_SWAP_H 1

/*
 * Simply pastes array size then ptr as args.
 * e.g. "5, ptr_to_arr_of_len_5"
 * (we do this a.. lot.. in here)
 */
#define ARRAY_SIZE_THEN_PTR(ptr) ARRAY_SIZE(ptr), ptr

struct active_block {
	struct tetris_block tetris_block;
	enum rotation rotation;
	/* position of block center */
	struct position position;
	struct position board_units[MAX_BLOCK_UNITS];
};

static const struct tetris_block tetris_block_null = {no_type, 0, NULL};

struct game_contents {
	int points;
	int lines_cleared;
	int auto_lower_count;
	int swap_h_block_count;
	int board[BOARD_HEIGHT][BOARD_WIDTH];
	unsigned int seed;
	struct tetris_block next_block;
	struct tetris_block hold_block;
	struct active_block *active_block;
	struct active_block *shadow_block;
};

struct srs_movement_descriptor {
	enum rotation start_rot;
	enum rotation end_rot;
	int test_count;
	const struct position *test_arr;
};

struct srs_movement_mode {
	int descriptor_count;
	const struct srs_movement_descriptor *descriptor_arr;
};

static const struct position srs_mode_standard_des_0_arr[] = {
    {0, 0}, {-1, 0}, {-1, 1}, {0, -2}, {-1, -2}};

static const struct position srs_mode_standard_des_1_arr[] = {
    {0, 0}, {1, 0}, {1, -1}, {0, 2}, {1, 2}};

static const struct position srs_mode_standard_des_2_arr[] = {
    {0, 0}, {1, 0}, {1, 1}, {0, -2}, {1, -2}};

static const struct position srs_mode_standard_des_3_arr[] = {
    {0, 0}, {-1, 0}, {-1, -1}, {0, 2}, {-1, 2}};

// none->right
// left->invert
static const struct position srs_mode_hero_des_0_arr[] = {
    {1, 0}, {-1, 0}, {2, 0}, {-1, -1}, {2, 2}};
// none->left
// right->invert
static const struct position srs_mode_hero_des_1_arr[] = {
    {0, -1}, {-1, -1}, {2, -1}, {-1, 1}, {2, -2}};
// invert->left
// right->none
static const struct position srs_mode_hero_des_2_arr[] = {
    {-1, 0}, {1, 0}, {-2, 0}, {1, 1}, {-2, -2}};
// left->none
// invert->right
static const struct position srs_mode_hero_des_3_arr[] = {
    {0, 1}, {1, 1}, {-2, 1}, {1, -1}, {-2, 2}};

static const struct srs_movement_descriptor srs_mode_standard_des_arr[] = {
    {none, right, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_0_arr)},
    {right, none, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_1_arr)},
    {right, invert, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_1_arr)},
    {invert, right, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_0_arr)},
    {invert, left, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_2_arr)},
    {left, invert, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_3_arr)},
    {left, none, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_3_arr)},
    {none, left, ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_2_arr)},
};

static const struct srs_movement_descriptor srs_mode_hero_des_arr[] = {
    {none, right, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_0_arr)},
    {right, none, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_2_arr)},
    {right, invert, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_1_arr)},
    {invert, right, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_3_arr)},
    {invert, left, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_2_arr)},
    {left, invert, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_0_arr)},
    {left, none, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_3_arr)},
    {none, left, ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_1_arr)},
};

static const struct srs_movement_mode srs_mode_standard = {
    ARRAY_SIZE_THEN_PTR(srs_mode_standard_des_arr)};

static const struct srs_movement_mode srs_mode_hero = {
    ARRAY_SIZE_THEN_PTR(srs_mode_hero_des_arr)};

static const struct position orange_block_offsets[] = {
    {0, 0}, {0, -1}, {1, -1}, {0, 1}};

static const struct position blue_block_offsets[] = {
    {0, 0}, {0, -1}, {-1, -1}, {0, 1}};

static const struct position cleve_block_offsets[] = {
    {0, 0}, {-1, 0}, {-1, 1}, {0, -1}};

static const struct position rhode_block_offsets[] = {
    {0, 0}, {-1, 0}, {-1, -1}, {0, 1}};

static const struct position teewee_block_offsets[] = {
    {0, 0}, {1, 0}, {0, 1}, {-1, 0}};

static const struct position hero_block_offsets[] = {
    {-1, 0}, {0, 0}, {1, 0}, {2, 0}};

static const struct position smashboy_block_offsets[] = {
    {0, 0}, {0, 1}, {1, 0}, {1, 1}};

static const struct tetris_block available_blocks[] = {
    {orange, 4, orange_block_offsets, &srs_mode_standard},
    {blue, 4, blue_block_offsets, &srs_mode_standard},
    {cleve, 4, cleve_block_offsets, &srs_mode_standard},
    {rhode, 4, rhode_block_offsets, &srs_mode_standard},
    {teewee, 4, teewee_block_offsets, &srs_mode_standard},
    {hero, 4, hero_block_offsets, &srs_mode_hero},
    {smashboy, 4, smashboy_block_offsets, NULL}};

#endif /* !TETRIS_GAME_PRIV_H */
