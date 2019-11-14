#include <stdlib.h>

#include "list.h"

struct st_list *list_create() {
	struct st_list *list = malloc(sizeof(struct st_list));
	return list;
}

static struct st_node *list_end_node(struct st_list *list) {
	struct st_node *target = list->head;
	if (target == 0)
		return target;
	while (target->next != 0)
		target = target->next;
	return target;
}

struct st_node *list_get(struct st_list *list, int index) {
	if (index >= list->length)
		return 0;
	struct st_node *node = list->head;
	for (int i = 0; i < index; i++)
		node = node->next;
	return node;
}

struct st_node *list_search(struct st_list *list, int (*match)(void *)) {
	struct st_node *node = list->head;
	while (node != 0)
		if (match(node->target))
			return node->target;
		else
			node = node->next;
	return 0;
}

void list_append(struct st_list *list, void *target) {
	// create a new node
	struct st_node *node = malloc(sizeof(struct st_node));
	node->target = target;
	node->next = 0;

	struct st_node *end = list_end_node(list);
	if (end == 0)
		list->head = node;
	else
		end->next = node;

	(list->length)++;
}

void list_free(struct st_list *list) {
	// first, free all the items of the list
	struct st_node *target;
	struct st_node *next = list->head;

	while (next != 0) {
		target = next;
		next = target->next;
		free(target);
	}

	// now, free the list itself
	free(list);
}

/**
 * returns true if the referee is the character 'h'
 */
// int
// h_matcher(void * val){
//   return 'h' == *(char*)val;
// }
//
// int
// main(void)
// {
//   struct st_list * list = list_create();
//   list_append(list, "hello");
//   list_search(list, h_matcher);
//   list_free(list);
// }
