#include "widgets.h"
#include "curses_combobox.h"
#include "curses_text_entry.h"
#include "generic.h"
#include "player.h"
#include <stdlib.h>
#include <string.h>

#define UINT_TO_INT(n) n < 32767 ? (int)n : (n > 32768 ? -(int)-n : -32768)

int ttviz_entry(char *user_input, char *label, int max_length) {
	int ch, _exit;
	CursesTextEntry *entry;

	erase();

	mvprintw(2, 2, label);

	if ((_exit = curses_text_field_create(&entry, stdscr, 3, 20, 3, 1,
	                                      max_length)) != EXIT_SUCCESS)
		return _exit;
	curses_text_field_refresh(entry);

	while ((ch = getch()) != '\n') {
		curses_text_field_feed(entry, ch);
		curses_text_field_refresh(entry);
	}

	strncpy(user_input, curses_text_field_value(entry), max_length);

	curses_text_field_destroy(entry);

	return EXIT_SUCCESS;
}

int edit_keybindings(ControlKeybindings *keybindings) {
	unsigned long i;
	int ch, _exit;
	CursesCombobox *entry;
	char *choices[] = {
	    "Left",      "Right", "Counter Clockwise", "Clockwise", "Soft Drop",
	    "Hard Drop", "Swap",  "Stop Game",
	};
	int *bindings[] = {
	    &keybindings->translate_left,
	    &keybindings->translate_right,
	    &keybindings->rotate_counterclockwise,
	    &keybindings->rotate_clockwise,
	    &keybindings->soft_drop,
	    &keybindings->hard_drop,
	    &keybindings->swap,
	    &keybindings->quit,
	};
	int num_choices = 8;

	initscr();
	start_color();
	cbreak();
	noecho();
	keypad(stdscr, TRUE);
	init_pair(1, COLOR_RED, COLOR_BLACK);
	curs_set(0); // disable cursor
	clear();

	mvprintw(0, 1, "Press enter to bind");
	mvprintw(1, 1, "Press ESC when done");

	// the sizing of this widget is pretty arbitrary
	if ((_exit = curses_combobox_create(&entry, stdscr, 10, 20, 4, 1,
	                                    choices, num_choices)) !=
	    EXIT_SUCCESS)
		exit(_exit);
	curses_combobox_refresh(entry);

	while (1) {
		// display the keybinding for the current selection
		i = curses_combobox_cursor_index(entry);
		unsigned int current_binding = *bindings[i];
		move(3, 0);
		clrtoeol();
		mvprintw(3, 1, "Binding for %s: %s", choices[i],
		         keyname(current_binding));
		if ((ch = getch()) == 27)
			break;
		// ignore space for single selection
		if (ch == ' ')
			continue;
		// after enter is pressed, allow the user to change the binding
		if (ch == '\n') {
			move(3, 0);
			clrtoeol();
			mvprintw(3, 1, "Press Any Key to Bind");
			*bindings[curses_combobox_cursor_index(entry)] =
			    getch();
		}
		// otherwise, feed the input to the combobox
		curses_combobox_feed(entry, ch);
		curses_combobox_refresh(entry);
	}

	return EXIT_SUCCESS;
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
