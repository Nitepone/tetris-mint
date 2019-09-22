#include <stdlib.h>

#include "list.h"
#include "player.h"

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
player_create()
{
	struct st_player * player = malloc(sizeof(struct st_player));
	list_append(player_list, player);
}
