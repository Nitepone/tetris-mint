#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "curses_text_entry.h"

#define UINT_TO_INT(n) n < 32767 ? (int)n : (n > 32768 ? -(int)-n : -32768)

struct curses_text_entry {
	WINDOW *parent_window;
	WINDOW *window;
	int height;
	int width;
	int starty;
	int startx;
	char *entered_text;
	unsigned int cursor_position;
	unsigned int max_length;
};

int curses_text_field_create(struct curses_text_entry **entry, WINDOW *parent,
                             int height, int width, int starty, int startx,
                             unsigned int max_length) {
	if (height < 3 || width < 3)
		return EXIT_FAILURE;

	*entry = calloc(1, sizeof(struct curses_text_entry));
	(*entry)->parent_window = parent;
	(*entry)->window = NULL;
	(*entry)->height = height;
	(*entry)->width = width;
	(*entry)->starty = starty;
	(*entry)->startx = startx;
	// allocate enough space for the string and an extra null byte
	(*entry)->entered_text = calloc(1, (*entry)->max_length + 1);
	if ((*entry)->entered_text == 0)
		return EXIT_FAILURE;
	(*entry)->cursor_position = 0;
	(*entry)->max_length = max_length;

	return EXIT_SUCCESS;
}

int curses_text_field_virtual_refresh(struct curses_text_entry *entry) {
	// equal to the width of the whole widget minus the border (2 chars)
	// plus 1 for the null byte
	char output_string[entry->width - 1];

	if (entry->window == NULL) {
		entry->window =
		    derwin(entry->parent_window, entry->height, entry->width,
		           entry->starty, entry->startx);
		// "If syncok is called with second argument TRUE then wsyncup
		// is called automatically whenever there is a change in the
		// window."
		//
		// This is necessary so that changes to our child window
		// propagate up to the root window.
		syncok(entry->window, TRUE);
	}

	strncpy(output_string, entry->entered_text, entry->width - 1);
	// ensure that the output string is null-terminated in the event that
	// the entered text is longer than the width of the text entry window
	output_string[entry->width] = 0;

	werase(entry->window);

	mvwprintw(entry->window, 1, 1, output_string);

	box(entry->window, 0, 0);

	// move the cursor to the character we are currently editing
	wmove(entry->window, 1, UINT_TO_INT(entry->cursor_position + 1u));
	// wcursyncup must be called so that the cursor is shown as it is in our
	// child window
	wcursyncup(entry->window);
	// make the cursor highly visible
	curs_set(2);

	wnoutrefresh(entry->window);

	return EXIT_SUCCESS;
}

int curses_text_field_refresh(struct curses_text_entry *entry) {
	int _exit;

	if ((_exit = curses_text_field_virtual_refresh(entry)) != EXIT_SUCCESS)
		return _exit;

	doupdate();
	return EXIT_SUCCESS;
}

static int curses_text_field_insert(struct curses_text_entry *entry,
                                    char input) {
	unsigned int i;
	char carry = 0, tmp = 0;

	// keep us in check!!
	if (entry->cursor_position == entry->max_length)
		return EXIT_SUCCESS;

	// if the cursor is not at the end, we need to shift the text before we
	// can insert our character
	if (entry->entered_text[entry->cursor_position] != 0) {
		i = entry->cursor_position;
		do {
			tmp = entry->entered_text[i];
			entry->entered_text[i] = carry;
			carry = tmp;
			i++;
		} while (carry != 0 && i < entry->max_length);
	} else {
		entry->entered_text[entry->cursor_position + 1] = 0;
	}

	entry->entered_text[entry->cursor_position] = input;
	entry->cursor_position += 1;

	return EXIT_SUCCESS;
}

static int curses_text_field_delete(struct curses_text_entry *entry) {
	unsigned int i;

	// if we are in position zero, we cannot delete any more
	if (entry->cursor_position == 0)
		return EXIT_SUCCESS;

	i = --entry->cursor_position;
	while (entry->entered_text[i] != 0) {
		entry->entered_text[i] = entry->entered_text[i + 1];
		i++;
	}

	return EXIT_SUCCESS;
}

int curses_text_field_feed(struct curses_text_entry *entry, int input) {
	switch (input) {
	case 1: // CTRL-A
		entry->cursor_position = 0;
		break;
	case 2: // CTRL-B
		if (entry->cursor_position > 0)
			entry->cursor_position--;
		break;
	case 5: // CTRL-E
		while (entry->entered_text[entry->cursor_position] != 0)
			entry->cursor_position++;
		break;
	case 6: // CTRL-F
		if (entry->entered_text[entry->cursor_position] != 0)
			entry->cursor_position++;
		break;
	case 8: // case 8 is required for backspace on windows
	case KEY_BACKSPACE:
		curses_text_field_delete(entry);
		break;
	case KEY_LEFT:
		entry->cursor_position = entry->cursor_position == 0
		                             ? entry->cursor_position
		                             : entry->cursor_position - 1;
		break;
	case KEY_RIGHT:
		entry->cursor_position =
		    entry->entered_text[entry->cursor_position] == 0
		        ? entry->cursor_position
		        : entry->cursor_position + 1;
		break;
	default:
		if (('a' <= input && input <= 'z') ||
		    ('A' <= input && input <= 'Z') ||
		    ('0' <= input && input <= '9'))
			curses_text_field_insert(entry, (char)input);
	}

	return EXIT_SUCCESS;
}

char *curses_text_field_value(struct curses_text_entry *entry) {
	return entry->entered_text;
}

int curses_text_field_destroy(struct curses_text_entry *entry) {
	// disable the cursor
	curs_set(0);

	// erase the window
	werase(entry->window);

	// delete the window
	delwin(entry->window);

	free(entry->entered_text);
	free(entry);

	return EXIT_SUCCESS;
}