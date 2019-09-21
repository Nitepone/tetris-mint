/*
 * tetris-game.h
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#ifndef TETRIS_GAME_H
#define TETRIS_GAME_H

enum block_type {
	orange,
	blue,
	cleve,
	rhode,
	teewee,
	smashboy,
	hero,
};
const static int BLOCK_TYPE_COUNT = 7;

enum rotation_angle {
	0,
	90,
	180,
	270,
};

struct position {
	int x;
	int y;
};

struct active_block {
	enum block_type block_type;
	enum rotation_angle rotation_angle;
	/* postition of block center */
	struct postition position;
};

const struct position center_position[BLOCK_TYPE_COUNT] = {
	{ 0, 1 }, // orange
	{ 1, 1 }, // blue
	{ 1, 1 }, // cleve
	{ 0, 1 }, // rhode
	{ 1, 0 }, // teewee
	// the following 2 pieces should not rely on this for rotation
	{ 0, 0 }, // hero
	{ 0, 0 }, // square
};

struct game_contents {
	int[10][24] board;
	struct active_block active_block;
};

/*
 * Lowers the block down the board by 1
 * @param - 0 if move is done by client, non-zero if by game
 * @return - 0 if piece is alive, else non-zero
 */
int lower_block (int if_forced);

/*
 * Translates a block left or right a unit
 * @param x - translates the block left if negative, right if positive
 * @return - 0 if piece moved, else non-zero
 */
int translate_block (int x);

/*
 * Rotates a block left or right
 * @param x - rotates the block left if negative, right if positive
 * @return - 0 if piece rotated, else non-zero
 */
int rotate_block (int x);

/*
 * Starts a game and takes a double pointer for game contents data
 * @return 0
 */
int start_game(struct **game_contents);


#endif /* !TETRIS_GAME_H */
