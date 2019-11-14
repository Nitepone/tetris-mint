#include <stdio.h>

#include "player.h"

int main(void) {
	player_init();
	struct st_player *george = player_create(0, "George");

	if (get_player_from_fd(0) == george)
		fprintf(stderr,
		        "Test 1: George stored and retrieved successfully.\n");
	else
		fprintf(stderr, "Test 1: Failed to retrieve player George\n");
}
