#include "tetris-game.h"
#include <pthread.h>

struct st_player {
	char * name;
	/* the current file descriptor */
	int fd;
	/* reference to the player's rendered board */
	struct game_view_data * view;
	/* player's game contents*/
	struct game_contents * contents;
	/* the player's clock thread */
	pthread_t game_clk_thread;
};

void player_init();

struct st_player * get_player_from_fd(int fd);

struct st_player * player_create(int fd, char * name);
