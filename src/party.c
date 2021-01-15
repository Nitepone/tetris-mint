#include <stdlib.h>

#include "list.h"
#include "party.h"
#include "player.h"

struct ttetris_party {
	List *players;
};

struct ttetris_party *ttetris_party_create() {
	struct ttetris_party *party = calloc(sizeof(struct ttetris_party), 1);
	party->players = list_create();
	return party;
};

void ttetris_party_player_add(struct ttetris_party *party, Player *player) {
	list_append(party->players, player);
	player->party = party;
}

List *ttetris_party_get_players(struct ttetris_party *party) {
	return party->players;
}

void ttetris_party_start(struct ttetris_party *party) {
	Player *player;

	for (int i = 0; i < party->players->length; i++) {
		player = (Player *)list_get(party->players, i);
		player_game_start(player);
	}
}
