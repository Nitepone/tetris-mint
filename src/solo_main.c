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
struct game_view_data *gvd;

void print_board() {
	int i, j, val;
	generate_game_view_data(&gvd, game_contents);
	printf("\e[1;1H\e[2J"); // clear posix
	printf("\n\n\n\n\n\n\n\n");
	for (i = (BOARD_HEIGHT - 1); i >= 0; i--) {
		printf("                    ");
		for (j = (BOARD_WIDTH - 1); j >= 0; j--) {
			val = gvd->board[i][j];
			printf("\e[0;3%dm", val);
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

	new_game(&game_contents);
	pthread_create(&thread, NULL, game_clock, (void *)game_contents);
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	while ((c = getchar()) != 'f') {
		switch (c) {
		case 'a':
			translate_block(1, game_contents);
			break;
		case 'd':
			translate_block(0, game_contents);
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
