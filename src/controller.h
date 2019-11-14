#ifndef _CONTROLLER_H
#define _CONTROLLER_H

/**
 * Define the control set interface. The actual implementation should be
 * provided by other files.
 */
typedef struct st_tetris_control_set {
	void (*translate)(int);
	void (*lower)(int);
	void (*rotate)(int);
	void (*drop)(void);
} TetrisControlSet;

/**
 * capture input from the keyboard and execute the correct function from the
 * provided control set
 */
void keyboard_input_loop(TetrisControlSet controls);

#endif
