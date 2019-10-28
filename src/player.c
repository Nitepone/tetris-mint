#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "list.h"
#include "tetris_game.h"
#include "player.h"

static struct st_list * player_list;

void
player_init()
{
        player_list = list_create();
}

void
*player_clock(void *input)
{
	struct st_player *player = (struct st_player*) input;
	while(1){
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
		lower_block(1, player->contents);

		// send board to player and clear socket if a failure occurs
		if (player->render(player->fd, player) == EXIT_FAILURE)
			player->fd = -1;

		// send board to opponent if one exists
		if( player->opponent )
			player->render(player->opponent->fd, player);
	}
	fprintf(stderr, "Player clock thread exited\n");
	return 0;
}

void
start_game(struct st_player *player)
{
	pthread_create(&player->game_clk_thread, NULL, player_clock,
			(void *)player);
}

struct st_player *
get_player_from_fd(int fd)
{
	struct st_player * player;
	for (int i=0;i<player_list->length;i++) {
		player = (struct st_player *)(list_get(player_list, i)->target);
		if (player->fd == fd)
			return player;
	}
	return 0;
}

/**
 * Get a player by name
 *
 * WARNING: this is a comparitively expensive operation.
 */
Player *
player_get_by_name(char * name)
{
	struct st_player * player;
	for (int i=0;i<player_list->length;i++) {
		player = (struct st_player *)(list_get(player_list, i)->target);
		fprintf(stderr, "player_get_by_name: Checking player '%s'\n", player->name);
		if (strcmp(player->name, name) == 0)
			return player;
	}
	fprintf(stderr, "player_get_by_name: No player found for '%s'\n", name);
	return NULL;
}

struct st_player *
player_create(int fd, char * name)
{
	struct st_player * player = malloc(sizeof(struct st_player));
	player->fd = fd;
	player->name = malloc(strlen(name) + 1);
	memcpy(player->name, name, strlen(name) + 1);
	player->opponent = NULL;
	player->contents = malloc(sizeof(struct game_contents));
	player->view = malloc(sizeof(struct game_view_data));
	list_append(player_list, player);
	fprintf(stderr, "player_create: Created player '%s'\n", player->name);
	player_get_by_name(player->name);
	fprintf(stderr, "player_create: There are now %d players\n", player_list->length);

	new_game(&player->contents);

	// start the game!
	start_game(player);

	return player;
}

void
player_set_opponent( Player * player, Player * opponent )
{
	if (opponent == NULL)
		return;

	player->opponent = opponent;
	opponent->opponent = player;

	fprintf(stderr, "player_set_opponent: %s and %s are now opponents\n", player->name, opponent->name);
}
