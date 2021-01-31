#include "widgets.h"
#include "curses_combobox.h"
#include "curses_text_entry.h"
#include "generic.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

#define UINT_TO_INT(n) n < 32767 ? (int)n : (n > 32768 ? -(int)-n : -32768)

void ttviz_entry(char *username, char *label, int max_length) {
	int ch, _exit;
	CursesTextEntry *entry;

	erase();

	mvprintw(2, 2, "Enter a Username:");

	if ((_exit = curses_text_field_create(&entry, stdscr, 3, 20, 3, 1,
	                                      max_length)) != EXIT_SUCCESS)
		exit(_exit);
	curses_text_field_refresh(entry);

	while ((ch = getch()) != '\n') {
		curses_text_field_feed(entry, ch);
		curses_text_field_refresh(entry);
	}

	strncpy(username, curses_text_field_value(entry), 7);

	curses_text_field_destroy(entry);
}

struct ttetris_widget_selection {
	/* options is a reference to the options provided by the caller */
	char **options;
	int num_options;
	int num_selected;
	int *indices;
};

void selection_destroy(WidgetSelection *selection) {
	free(selection->indices);
	free(selection);
}

/**
 * Select from a given number of options
 */
struct ttetris_widget_selection *ttviz_select(char **options, int num_options,
                                              char *desc,
                                              int is_single_selection) {
	int i, ch, _exit;
	CursesCombobox *entry;

	clear();

	if (!is_single_selection) {
		mvprintw(1, 1, "Use <SPACE> to select or unselect an item");
		mvprintw(2, 1, "Use <ENTER> to confirm selection");
	}

	mvprintw(4, 2, desc);

	// the sizing of this widget is pretty arbitrary
	if ((_exit = curses_combobox_create(&entry, stdscr, 7, 20, 5, 1,
	                                    options, num_options)) !=
	    EXIT_SUCCESS)
		exit(_exit);
	curses_combobox_refresh(entry);

	while ((ch = getch()) != '\n') {
		// ignore space for single selection
		if (is_single_selection && ch == ' ')
			continue;
		curses_combobox_feed(entry, ch);
		curses_combobox_refresh(entry);
	}

	int *is_option_selected = curses_combobox_value(entry);

	WidgetSelection *w_selection = malloc(sizeof(WidgetSelection));
	w_selection->options = options;
	w_selection->num_options = num_options;

	if (is_single_selection) {
		w_selection->num_selected = 1;
		w_selection->indices =
		    calloc(sizeof(int), w_selection->num_selected);
		w_selection->indices[0] =
		    UINT_TO_INT(curses_combobox_cursor_index(entry));
	} else {
		// count the number of items
		w_selection->num_selected = 0;
		for (i = 0; i < num_options; ++i)
			if (is_option_selected[i] == TRUE)
				w_selection->num_selected += 1;

		// store the selected indices
		w_selection->indices =
		    calloc(sizeof(int), w_selection->num_selected);
		int j = 0;
		for (i = 0; i < num_options; ++i)
			if (is_option_selected[i] == TRUE)
				w_selection->indices[j++] = i;
	}

	curses_combobox_destroy(entry);

	return w_selection;
}

int selection_to_index(WidgetSelection *selection) {
	return selection->indices[0];
}

StringArray *selection_to_string_array(WidgetSelection *selection) {
	StringArray *arr =
	    string_array_create(selection->num_selected, PLAYER_NAME_MAX_CHARS);

	for (int i = 0; i < selection->num_selected; ++i)
		string_array_set_item(
		    arr, i, selection->options[selection->indices[i]]);

	return arr;
}
