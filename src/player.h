#ifndef PLAYER_H
#define PLAYER_H

#include "tetris_game.h"
#include <pthread.h>

typedef struct st_player {
	char * name;
	/* opponent */
	struct st_player * opponent;
	/* the current file descriptor */
	int fd;
	/* reference to the player's rendered board */
	struct game_view_data * view;
	/* player's game contents*/
	struct game_contents * contents;
	/* the player's clock thread */
	pthread_t game_clk_thread;
	/* render function */
	int (*render)(int socket_fd, struct st_player *);
} Player;

void player_init();

struct st_player * get_player_from_fd(int fd);

struct st_player * player_create(int fd, char * name);

Player * player_get_by_name(char * name);

void player_set_opponent( Player * player, Player * opponent );

#endif
