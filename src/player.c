#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "event.h"
#include "generic.h"
#include "list.h"
#include "log.h"
#include "player.h"
#include "tetris_game.h"

static struct st_list *player_list;

void player_init() { player_list = list_create(); }

void *player_clock(void *input) {
	struct st_player *player = (struct st_player *)input;
	fprintf(logging_fp, "player_clock: thread started\n");
	do {
		nanosleep((const struct timespec[]){{0, 500000000L}}, NULL);
		lower_block(player->contents, 1);

		// if the player has a party, send the board to all players
		if (player->party) {
			List *party_members =
			    ttetris_party_get_players(player->party);
			for (int i = 0; i < party_members->length; i++)
				if (player->render(
				        ((Player *)list_get(party_members, i))
				            ->fd,
				        player) == EXIT_FAILURE)
					((Player *)list_get(party_members, i))
					    ->fd = -1;
		}
		// otherwise, just send the board to the player
		else {
			if (player->render(player->fd, player) == EXIT_FAILURE)
				player->fd = -1;
		}

	} while (game_over(player->contents) == 0);
	fprintf(logging_fp, "player_clock: thread exiting\n");
	return 0;
}

void player_game_start(struct st_player *player) {
	pthread_create(&player->game_clk_thread, NULL, player_clock,
	               (void *)player);
}

void player_game_stop(struct st_player *player) {
	pthread_cancel(player->game_clk_thread);
}

struct st_player *get_player_from_fd(int fd) {
	struct st_player *player;
	for (int i = 0; i < player_list->length; i++) {
		player = (struct st_player *)list_get(player_list, i);
		if (player->fd == fd)
			return player;
	}
	return 0;
}

StringArray *player_names(int exclude_in_game) {
	int player_index, name_array_index;

	StringArray *arr =
	    string_array_create(player_list->length, PLAYER_NAME_MAX_CHARS);

	name_array_index = 0;
	for (player_index = 0; player_index < player_list->length;
	     player_index++) {
		Player *player =
		    (struct st_player *)list_get(player_list, player_index);
		// skip players with no active socket file descriptor
		if (player->fd == -1)
			continue;
		// skip players that are "in-game"
		if (exclude_in_game && player->party != NULL)
			continue;
		string_array_set_item(arr, name_array_index++, player->name);
	}

	// downsize the string array
	string_array_resize(arr, name_array_index);

	return arr;
}

/**
 * Get a player by name
 *
 * WARNING: this is a comparitively expensive operation.
 */
Player *player_get_by_name(char *name) {
	struct st_player *player;
	for (int i = 0; i < player_list->length; i++) {
		player = (struct st_player *)list_get(player_list, i);
		fprintf(logging_fp,
		        "player_get_by_name: Checking player '%s'\n",
		        player->name);
		if (strcmp(player->name, name) == 0)
			return player;
	}
	fprintf(logging_fp, "player_get_by_name: No player found for '%s'\n",
	        name);
	return NULL;
}

struct st_player *player_create(int fd, char *name) {
	struct st_player *player = malloc(sizeof(struct st_player));
	player->fd = fd;
	player->name = malloc(strlen(name) + 1);
	memcpy(player->name, name, strlen(name) + 1);
	player->game_start_event = ttetris_event_create();
	player->party = NULL;
	/* contents will be initialized by new_game */
	player->contents = NULL;
	player->view = malloc(sizeof(struct game_view_data));
	list_append(player_list, player);
	fprintf(logging_fp, "player_create: Created player '%s'\n",
	        player->name);
	player_get_by_name(player->name);
	fprintf(logging_fp, "player_create: There are now %d players\n",
	        player_list->length);

	new_game(&player->contents);

	return player;
}
