#ifndef _CONTROLLER_H
#define _CONTROLLER_H

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
void keyboard_input_loop(TetrisControlSet controls, void *context);

#endif
