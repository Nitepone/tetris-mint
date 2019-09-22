#include <stdlib.h>
#include <stdio.h>

#include "list.h"
#include "player.h"
#include "tetris-game.h"

static struct st_list * player_list;

void
player_init()
{
        player_list = list_create();
}

struct st_player *
get_player_from_fd(int fd)
{
	struct st_player * player;
	for (int i=0;i<player_list->length;i++){
		player = (struct st_player *)list_get(player_list, i);
		if (player->fd == fd)
		return player;
	}
	return 0;
}

void
player_create(int fd, char * name)
{
	struct st_player * player = malloc(sizeof(struct st_player));
	player->fd = fd;
	player->name = name;
	player->contents = malloc(sizeof(struct game_contents));
	player->view = malloc(sizeof(struct game_view_data));
	list_append(player_list, player);
	fprintf(stderr, "There are now %d players", player_list->length);

	new_game(&player->contents);
	generate_game_view_data(&player->view, player->contents);
}
