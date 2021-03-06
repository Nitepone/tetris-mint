#ifndef _CONTROLLER_H
#define _CONTROLLER_H

typedef struct control_keybindings {
	int hard_drop;
	int soft_drop;
	int translate_left;
	int translate_right;
	int rotate_clockwise;
	int rotate_counterclockwise;
	int swap;
	int quit;
} ControlKeybindings;

/**
 * Define the control set interface. The actual implementation should be
 * provided by other files.
 */
typedef struct st_tetris_control_set {
	void (*translate)(void *context, int);
	void (*lower)(void *context);
	void (*rotate)(void *context, int);
	void (*drop)(void *context);
	void (*swap_hold)(void *context);
} TetrisControlSet;

/**
 * capture input from the keyboard and execute the correct function from the
 * provided control set
 * @param controls struct containing methods to be called in response to
 * keypress events
 * @param context pointer to an object that will be passed to the methods
 * in the given control set
 */
void keyboard_input_loop(TetrisControlSet controls,
                         ControlKeybindings keybindings, void *context);

ControlKeybindings default_keybindings(void);

#endif
