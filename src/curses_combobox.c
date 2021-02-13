#include <curses.h>
#include <stdlib.h>
#include <string.h>

#include "curses_combobox.h"

#define UINT_TO_INT(n) n < 32767 ? (int)n : (n > 32768 ? -(int)-n : -32768)

struct curses_combobox {
	WINDOW *parent_window;
	WINDOW *window;
	int height;
	int width;
	int starty;
	int startx;
	unsigned int first_shown_option;
	unsigned int cursor_position;
	char **options;
	unsigned int option_count;
	// is_index_selected is a pointer to an array that maps to the
	// user-provided options
	int *is_option_selected;
};

int curses_combobox_create(struct curses_combobox **entry, WINDOW *parent,
                           int height, int width, int starty, int startx,
                           char **options, unsigned int option_count) {
	if (height < 3 || width < 3)
		return EXIT_FAILURE;

	*entry = calloc(1, sizeof(struct curses_combobox));
	(*entry)->parent_window = parent;
	(*entry)->window = NULL;
	(*entry)->height = height;
	(*entry)->width = width;
	(*entry)->starty = starty;
	(*entry)->startx = startx;

	(*entry)->options = options;
	(*entry)->option_count = option_count;
	(*entry)->first_shown_option = 0;
	(*entry)->cursor_position = 0;

	(*entry)->is_option_selected = calloc(option_count, sizeof(int));

	return EXIT_SUCCESS;
}

int curses_combobox_virtual_refresh(struct curses_combobox *entry) {

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

	werase(entry->window);

	if (entry->cursor_position < entry->first_shown_option)
		entry->first_shown_option = entry->cursor_position;
	else if (entry->cursor_position >
	         entry->first_shown_option + entry->height - 3)
		entry->first_shown_option =
		    entry->cursor_position - entry->height + 3;

	int i = 0;
	unsigned int j = entry->first_shown_option;

	while (j < entry->option_count && i < entry->height - 2) {
		mvwprintw(entry->window, 1 + i, 1, "%c%s",
		          entry->is_option_selected[j] ? '*' : ' ',
		          entry->options[j]);
		// This call must follow the printing of the text
		if (entry->cursor_position == j)
			mvwchgat(entry->window, 1 + i, 1, entry->width - 2,
			         A_STANDOUT, 0, NULL);
		else
			mvwchgat(entry->window, 1 + i, 1, entry->width - 2,
			         A_NORMAL, 0, NULL);
		i++;
		j++;
	}

	box(entry->window, 0, 0);

	wnoutrefresh(entry->window);

	return EXIT_SUCCESS;
}

int curses_combobox_refresh(struct curses_combobox *entry) {
	int _exit;

	if ((_exit = curses_combobox_virtual_refresh(entry)) != EXIT_SUCCESS)
		return _exit;

	doupdate();
	return EXIT_SUCCESS;
}

int curses_combobox_feed(struct curses_combobox *entry, int input) {
	switch (input) {
	case KEY_LEFT:
	case KEY_UP:
		entry->cursor_position = entry->cursor_position == 0
		                             ? entry->cursor_position
		                             : entry->cursor_position - 1;
		break;
	case KEY_RIGHT:
	case KEY_DOWN:
		entry->cursor_position =
		    entry->cursor_position < entry->option_count - 1
		        ? entry->cursor_position + 1
		        : entry->cursor_position;
		break;
	case ' ':
		entry->is_option_selected[entry->cursor_position] =
		    entry->is_option_selected[entry->cursor_position] == FALSE
		        ? TRUE
		        : FALSE;
	default:
		return EXIT_SUCCESS;
	}

	return EXIT_SUCCESS;
}

unsigned int curses_combobox_cursor_index(struct curses_combobox *entry) {

	return entry->cursor_position;
}

int *curses_combobox_value(struct curses_combobox *entry) {

	return entry->is_option_selected;
}

int curses_combobox_destroy(struct curses_combobox *entry) {
	// erase the window
	werase(entry->window);

	// delete the window
	delwin(entry->window);

	free(entry);

	return EXIT_SUCCESS;
}