#include "render.h"
#include "tetris_game.h"

// global variables and static variables are automatically initialized to zero
struct game_view_data view;

int main(void) {
	view.lines_cleared = 1;
	view.points = 100;
	int(*board)[BOARD_WIDTH] = view.board;

	// T piece
	board[17][5] = 1;
	board[17][6] = 1;
	board[17][7] = 1;
	board[16][6] = 1;

	// L piece
	board[1][0] = 3;
	board[0][0] = 3;
	board[0][1] = 3;
	board[0][2] = 3;

	// line piece
	board[0][6] = 4;
	board[0][7] = 4;
	board[0][8] = 4;
	board[0][9] = 4;

	char *names[2];
	names[0] = "Elliot";
	names[1] = "Tyler";

	render_init(2, names);
	render_game_view_data(names[0], &view);
	render_game_view_data(names[1], &view);
	render_message("Welcome to Tetris! Press q to quit.");

	// wait for 'q' key
	while (getch() != 'q') {
	}

	render_close();

	return 0;
}
