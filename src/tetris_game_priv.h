/*
 * tetris-game-priv.h
 * Copyright (C) 2019-2021 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TETRIS_GAME_PRIV_H
#define TETRIS_GAME_PRIV_H

#include <stddef.h>

#define MAX_BLOCK_UNITS 4
#define BLOCK_START_POSITION                                                   \
	{ 4, 21 }

#define MAX_AUTO_LOWER 3
#define MAX_SWAP_H 1

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

enum rotation {
	none,
	right,
	invert,
	left,
};
#define ROT_COUNT 4

enum block_rotation_type { no_rotate, center_based, corner_based };

struct tetris_block {
	enum block_type type;
	enum block_rotation_type rotation_type;
	unsigned int cell_count;
	const struct position *position_offsets;
};

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
    {0, -1}, {0, 0}, {0, 1}, {0, 2}};

static const struct position smashboy_block_offsets[] = {
    {0, 0}, {0, 1}, {1, 0}, {1, 1}};

static const struct tetris_block available_blocks[] = {
    {orange, center_based, 4, orange_block_offsets},
    {blue, center_based, 4, blue_block_offsets},
    {cleve, center_based, 4, cleve_block_offsets},
    {rhode, center_based, 4, rhode_block_offsets},
    {teewee, center_based, 4, teewee_block_offsets},
    {hero, corner_based, 4, hero_block_offsets},
    {smashboy, corner_based, 4, smashboy_block_offsets}};

static const struct tetris_block tetris_block_null = {no_type, no_rotate, 0,
                                                      NULL};

struct active_block {
	struct tetris_block tetris_block;
	enum rotation rotation;
	/* position of block center */
	struct position position;
	struct position board_units[MAX_BLOCK_UNITS];
};

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

#endif /* !TETRIS_GAME_PRIV_H */
