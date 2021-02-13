#include "controller.h"
#include <curses.h>

void keyboard_input_loop(TetrisControlSet controls,
                         ControlKeybindings keybindings, void *context) {
	int ch;
	while ((ch = getch()) != keybindings.quit) {
		if (ch == keybindings.translate_left)
			controls.translate(context, 0);
		else if (ch == keybindings.translate_right)
			controls.translate(context, 1);
		else if (ch == keybindings.rotate_counterclockwise)
			controls.rotate(context, 0);
		else if (ch == keybindings.rotate_clockwise)
			controls.rotate(context, 1);
		else if (ch == keybindings.soft_drop)
			controls.lower(context);
		else if (ch == keybindings.hard_drop)
			controls.drop(context);
		else if (ch == keybindings.swap)
			controls.swap_hold(context);
	}
}

ControlKeybindings default_keybindings(void) {
	return (ControlKeybindings){
	    .hard_drop = 'w',
	    .soft_drop = 's',
	    .translate_left = 'a',
	    .translate_right = 'd',
	    .rotate_clockwise = 'e',
	    .rotate_counterclockwise = 'q',
	    .swap = 'c',
	    .quit = 27, // ESCAPE
	};
}