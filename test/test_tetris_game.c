/*
 * test_tetris_game.c
 * Copyright (C) 2020-2021 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include "tetris_game.h"
#include "unity.h"

/* Is run before every test, put unit init calls here. */
void setUp(void) {}

/* Is run after every test, put unit clean-up calls here. */
void tearDown(void) {}

void test_start_game(void) {
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_game_view(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	// Generate game view and inspect contents
	TEST_ASSERT_EQUAL_INT(0, generate_game_view_data(gc, &gvd));
	TEST_ASSERT_EQUAL_INT(0, gvd->points);
	TEST_ASSERT_EQUAL_INT(0, gvd->lines_cleared);
	TEST_ASSERT_EQUAL_INT(hero, gvd->next_block);
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_hold_block(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	// Check current block
	TEST_ASSERT_EQUAL_INT(teewee, gc->active_block->tetris_block.type);
	// Swap block to hold
	TEST_ASSERT_EQUAL_INT(0, swap_hold_block(gc));
	// Check current block
	TEST_ASSERT_EQUAL_INT(hero, gc->active_block->tetris_block.type);
	// Check that GVD contains correct block
	TEST_ASSERT_EQUAL_INT(0, generate_game_view_data(gc, &gvd));
	TEST_ASSERT_EQUAL_INT(teewee, gvd->hold_block);
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_hold_block_lock(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	// Swap block to hold
	TEST_ASSERT_EQUAL_INT(0, swap_hold_block(gc));
	// Attempt a second swap
	TEST_ASSERT_EQUAL_INT(-1, swap_hold_block(gc));
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_left_boundary(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	TEST_ASSERT_EQUAL_INT(teewee, gc->active_block->tetris_block.type);
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	// This next move should hit boundary
	TEST_ASSERT_EQUAL_INT(-1, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_right_boundary(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	TEST_ASSERT_EQUAL_INT(teewee, gc->active_block->tetris_block.type);
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	// This next move should hit boundary
	TEST_ASSERT_EQUAL_INT(-1, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

void test_clear_line(void) {
	struct game_view_data *gvd = NULL;
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_INT(0, new_seeded_game(&gc, 0));
	// Position and drop block 1
	TEST_ASSERT_EQUAL_INT(teewee, gc->active_block->tetris_block.type);
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_left(gc));
	TEST_ASSERT_EQUAL_INT(0, hard_drop(gc));
	// Position and drop block 2
	TEST_ASSERT_EQUAL_INT(hero, gc->active_block->tetris_block.type);
	TEST_ASSERT_EQUAL_INT(0, rotate_block(gc, 1));
	TEST_ASSERT_EQUAL_INT(0, hard_drop(gc));
	// Position and drop block 3
	TEST_ASSERT_EQUAL_INT(teewee, gc->active_block->tetris_block.type);
	TEST_ASSERT_EQUAL_INT(0, translate_block_right(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_right(gc));
	TEST_ASSERT_EQUAL_INT(0, translate_block_right(gc));
	TEST_ASSERT_EQUAL_INT(0, hard_drop(gc));
	// Count Points and Lines
	TEST_ASSERT_EQUAL_INT(0, generate_game_view_data(gc, &gvd));
	TEST_ASSERT_EQUAL_INT(100, gvd->points);
	TEST_ASSERT_EQUAL_INT(1, gvd->lines_cleared);
	TEST_ASSERT_EQUAL_INT(0, destroy_game(&gc));
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_start_game);
	RUN_TEST(test_game_view);
	RUN_TEST(test_hold_block);
	RUN_TEST(test_hold_block_lock);
	RUN_TEST(test_left_boundary);
	RUN_TEST(test_right_boundary);
	RUN_TEST(test_clear_line);
	return UNITY_END();
}
