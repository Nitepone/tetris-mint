#ifndef PLAYER_H
#define PLAYER_H

#include <pthread.h>

#include "event.h"
#include "generic.h"
#include "party.h"
#include "tetris_game.h"

// does not count the zero-byte / null-terminator
#define PLAYER_NAME_MAX_CHARS 15

// forward-definition of TetrisParty so that we can do a circular import with
// "party.h"
typedef struct ttetris_party TetrisParty;

typedef struct st_player Player;

struct st_player {
	char *name;
	/* (optional) party */
	TetrisParty *party;
	/* (optional) game start event */
	TetrisEvent *game_start_event;
	/* the current file descriptor */
	int fd;
	/* reference to the player's rendered board */
	struct game_view_data *view;
	/* player's game contents*/
	struct game_contents *contents;
	/* the player's clock thread */
	pthread_t game_clk_thread;
	/* render function */
	int (*render)(int socket_fd, struct st_player *);
};

void player_init();

struct st_player *get_player_from_fd(int fd);

struct st_player *player_create(int fd, char *name);

StringArray *player_names(int exclude_in_game);

void player_game_start(struct st_player *player);

void player_game_stop(struct st_player *player);

Player *player_get_by_name(char *name);

#endif
