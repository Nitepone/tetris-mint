#include "controller.h"
#include <ncurses.h>

void keyboard_input_loop(TetrisControlSet controls) {
	char ch;
	while ((ch = getch()) != 27) {
		switch (ch) {
		case 'a':
			controls.translate(0);
			break;
		case 'w':
			controls.drop();
			break;
		case 's':
			controls.lower(1);
			break;
		case 'd':
			controls.translate(1);
			break;
		case 'q':
			controls.rotate(0);
			break;
		case 'e':
			controls.rotate(1);
			break;
		}
	}
}
