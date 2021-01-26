/*
 * test-main.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <pthread.h>
#include <stdio.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "tetris_game.h"

struct game_contents *game_contents;
struct game_view_data *gvd = NULL;

void print_board() {
	int i, j, val;
	generate_game_view_data(&gvd, game_contents);
	printf("\e[1;1H\e[2J"); // clear posix
	printf("\n\n\n\n\n\n\n\n");
	for (i = (BOARD_HEIGHT - 1); i >= 0; i--) {
		printf("                    ");
		printf("\e[40;1m");
		for (j = (BOARD_WIDTH - 1); j >= 0; j--) {
			val = gvd->board[i][j];
			switch ((enum block_type)val) {
				case shadow:
					printf("\e[37;1m");
					break;
				case empty:
					printf("\e[30;1m");
					break;
				case smashboy:
					printf("\e[93;1m");
					break;
				case hero:
					printf("\e[96;1m");
					break;
				case orange:
					printf("\e[33;1m");
					break;
				case blue:
					printf("\e[34;1m");
					break;
				case cleve:
					printf("\e[32;1m");
					break;
				case rhode:
					printf("\e[31;1m");
					break;
				case teewee:
					printf("\e[35;1m");
					break;
			};
			printf("██");
		}
		printf("\n\e[0;0m");
	}
	printf("                    ");
	printf("Points: %d   Lines: %d", game_contents->points,
	       game_contents->lines_cleared);
	printf("\n");
}

void *game_clock(void *input) {
	while (1) {
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
		lower_block(1, game_contents);
		print_board();
	}
	return 0;
}

int main() {
	char c = 0;
	static struct termios oldt, newt;
	pthread_t thread;

	new_seeded_game(&game_contents, 0);
	pthread_create(&thread, NULL, game_clock, (void *)game_contents);
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	while ((c = getchar()) != 'f') {
		switch (c) {
		case 'a':
			translate_block_left(game_contents);
			break;
		case 'd':
			translate_block_right(game_contents);
			break;
		case 'q':
			rotate_block(1, game_contents);
			break;
		case 'e':
			rotate_block(0, game_contents);
			break;
		case 's':
			lower_block(0, game_contents);
			break;
		case 'w':
			hard_drop(game_contents);
			break;
		case 'c':
			swap_hold_block(game_contents);
			break;
		}
		print_board();
	}
	return 0;
}
