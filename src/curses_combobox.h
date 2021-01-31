#ifndef TTETRIS_CURSES_COMBOBOX_H
#define TTETRIS_CURSES_COMBOBOX_H

#include <curses.h>

typedef struct curses_combobox CursesCombobox;

int curses_combobox_create(CursesCombobox **entry, WINDOW *parent, int height,
                           int width, int starty, int startx, char **options,
                           unsigned int option_count);

int curses_combobox_virtual_refresh(CursesCombobox *entry);

int curses_combobox_refresh(CursesCombobox *entry);

int curses_combobox_feed(CursesCombobox *entry, int input);

unsigned int curses_combobox_cursor_index(CursesCombobox *entry);

int *curses_combobox_value(CursesCombobox *entry);

int curses_combobox_destroy(CursesCombobox *entry);

#endif // TTETRIS_CURSES_COMBOBOX_H
