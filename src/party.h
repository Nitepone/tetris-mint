/**
 * A tetris party is a group of players that are going to play together.
 *
 * For example, a player struct may be allocated on the server containing
 * three players. Each of these three players will be able to see their
 * own board, as well as the boards for the other two players.
 */
#ifndef TTETRIS_PARTY_H
#define TTETRIS_PARTY_H

#include "list.h"
#include "player.h"

typedef struct ttetris_party TetrisParty;

TetrisParty *ttetris_party_create();

// forward-definition of Player so that we can do a circular import with
// "player.h"
typedef struct st_player Player;

void ttetris_party_player_add(TetrisParty *party, Player *player);

/**
 * start the tetris game for all players in the party
 *
 * ie.
 * - start a thread for each player that updates the state of their game board
 * - send a message over the network to each player indicating the game has
 *   started
 * - etc.
 * @param party
 */
void ttetris_party_start(TetrisParty *party);

List *ttetris_party_get_players(TetrisParty *party);

#endif // TTETRIS_PARTY_H
