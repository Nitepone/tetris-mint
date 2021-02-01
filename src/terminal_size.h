#ifndef _TERMINAL_SIZE_H
#define _TERMINAL_SIZE_H

#include <stdint.h>

typedef struct terminal_size {
	uint16_t rows;
	uint16_t columns;
} TerminalSize;

TerminalSize get_terminal_size();

#endif