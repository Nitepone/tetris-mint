#ifndef TTETRIS_WIDGETS_H
#define TTETRIS_WIDGETS_H

#include "generic.h"

typedef struct ttetris_widget_selection WidgetSelection;

int selection_to_index(WidgetSelection *selection);
StringArray *selection_to_string_array(WidgetSelection *selection);
void selection_destroy(WidgetSelection *selection);

void ttviz_entry(char *username, char *label, int max_length);
WidgetSelection *ttviz_select(char **options, int num_options, char *desc,
                              int is_single_selection);

#endif // TTETRIS_WIDGETS_H