#include "controller.h"
#include <curses.h>

void keyboard_input_loop(TetrisControlSet controls, void *context) {
	char ch;
	while ((ch = getch()) != 27) {
		switch (ch) {
		case 'a':
			controls.translate(context, 0);
			break;
		case 'w':
			controls.drop(context);
			break;
		case 's':
			controls.lower(context);
			break;
		case 'd':
			controls.translate(context, 1);
			break;
		case 'q':
			controls.rotate(context, 0);
			break;
		case 'e':
			controls.rotate(context, 1);
			break;
		case 'c':
			controls.swap_hold(context);
			break;
		}
	}
}
