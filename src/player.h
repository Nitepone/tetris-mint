#include "tetris-game.h"

struct st_player {
	char * name;
	/* the current file descriptor */
	int fd;
	/* reference to the player's rendered board */
	struct game_view_data * view;
	/* player's game contents*/
	struct game_contents * contents;
};

void player_init();

struct st_player * get_player_from_fd(int fd);

void player_create();
