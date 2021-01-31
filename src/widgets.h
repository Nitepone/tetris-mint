#ifndef TTETRIS_WIDGETS_H
#define TTETRIS_WIDGETS_H

#include "controller.h"
#include "generic.h"

typedef struct ttetris_widget_selection WidgetSelection;

int selection_to_index(WidgetSelection *selection);
StringArray *selection_to_string_array(WidgetSelection *selection);
void selection_destroy(WidgetSelection *selection);

/**
 * full-screen synchronous text entry using curses
 * @param user_input pointer to buffer where the entered text will be written
 * @param label text to display to user above the input field
 * @param max_length size of the user_input buffer
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int ttviz_entry(char *user_input, char *label, int max_length);
WidgetSelection *ttviz_select(char **options, int num_options, char *desc,
                              int is_single_selection);
int edit_keybindings(ControlKeybindings *keybindings);

#endif // TTETRIS_WIDGETS_H