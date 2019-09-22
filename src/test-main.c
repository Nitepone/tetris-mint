/*
 * test-main.c
 * Copyright (C) 2019 nitepone <admin@night.horse>
 *
 * Distributed under terms of the MIT license.
 */

#include <termios.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <unistd.h>

#include "tetris-game.h"

struct game_contents *game_contents;
struct game_view_data *gvd;

void print_board() {
	int i, j, val;
	generate_game_view_data(&gvd, game_contents);
	printf("\e[1;1H\e[2J"); // clear posix
	printf("\n\n\n\n\n\n\n\n");
	for (i = 23; i >= 0; i--) {
		printf("                    ");
		for (j = 9; j >= 0; j--) {
			val = gvd->board[i][j];
			printf("\e[0;3%dm",val);
			printf("██");
		}
		printf("\n\e[0;0m");
	}
	printf("\n\n\n\n\n\n\n\n");
	printf("Points: %d   Lines: %d", game_contents->points, game_contents->lines_cleared);
	printf("\n");
}

void* game_clock(void *input) {
	while(1) {
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
		lower_block(1, game_contents);
		print_board();
	}
	return 0;
}

int main() {
//	int i;
	char c=0;
	pthread_t thread;

	new_game(&game_contents);
	//printf("Created Game: %d %d\n", game_contents->board[0][0], game_contents->active_block);
	print_board();
	pthread_create(&thread, NULL, game_clock, (void *)game_contents);


	static struct termios oldt, newt;
	tcgetattr( STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON);
	tcsetattr( STDIN_FILENO, TCSANOW, &newt);

	while((c = getchar()) != 'f') {
		switch (c) {
			case 'a':
				translate_block(1, game_contents);
				break;
			case 'd':
				translate_block(0, game_contents);
				break;
			case 'q':
				rotate_block(0, game_contents);
				break;
			case 'e':
				rotate_block(1, game_contents);
				break;
			case 's':
				lower_block(0, game_contents);
				break;
		}
		print_board();
	}
#if 0
	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	lower_block(0, game_contents);
	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	lower_block(0, game_contents);

	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	translate_block(0, game_contents);
	for (i = 0; i < 19; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	translate_block(1, game_contents);
	for (i = 0; i < 20; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	translate_block(1, game_contents);
	translate_block(1, game_contents);
	for (i = 0; i < 20; i++)
		lower_block(1, game_contents);
	lower_block(0, game_contents);

	/*
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	rotate_block(1, game_contents);
	*/
	rotate_block(0, game_contents);

	//rotate_block(1, game_contents);
#endif
	return 0;
}
