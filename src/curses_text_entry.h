/**
 * Note: The functions in this file are loosely named and modelled after the
 * curses functions for window manipulation.
 */
#ifndef TTETRIS_CURSES_TEXT_ENTRY_H
#define TTETRIS_CURSES_TEXT_ENTRY_H

#include <curses.h>
#include <stdlib.h>

typedef struct curses_text_entry CursesTextEntry;

/**
 * allocates and initializes a new CursesTextEntry
 *
 * The direct pointee of entry will be a pointer to the CursesTextEntry
 * allocated by this function.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int curses_text_field_create(CursesTextEntry **entry, WINDOW *parent,
                             int height, int width, int starty, int startx,
                             unsigned int max_length);

/**
 * Copies the updated text entry to the curses "virtual screen", but does not
 * call doupdate (man wrefresh)
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int curses_text_field_virtual_refresh(CursesTextEntry *entry);

/**
 * Update the curses "virtual screen" to reflect the current state of the text
 * entry widget, and flush the result to the terminal screen.
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int curses_text_field_refresh(CursesTextEntry *entry);

/**
 * Feed an input character to the text field
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int curses_text_field_feed(CursesTextEntry *entry, int input);

/**
 * Return the current value of the text entry
 *
 * The underlying memory may change, so you will want to memcpy the returned
 * pointer.
 */
char *curses_text_field_value(CursesTextEntry *entry);

/**
 * Destroy the text field
 *
 * @return EXIT_SUCCESS or EXIT_FAILURE
 */
int curses_text_field_destroy(struct curses_text_entry *entry);

#endif // TTETRIS_CURSES_TEXT_ENTRY_H
