/*
 * test_tetris_game.c
 * Copyright (C) 2020 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include "tetris_game.h"
#include "unity.h"

void setUp (void) {} /* Is run before every test, put unit init calls here. */
void tearDown (void) {} /* Is run after every test, put unit clean-up calls here. */

void test_basic(void) {
	struct game_contents *gc = NULL;
	TEST_ASSERT_EQUAL_HEX8(0, new_seeded_game(&gc, 0));
}

int main(void) {
	UNITY_BEGIN();
	RUN_TEST(test_basic);
	return UNITY_END();
}



