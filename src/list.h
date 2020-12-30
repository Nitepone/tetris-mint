#ifndef TTETRIS_LIST_H
#define TTETRIS_LIST_H

struct st_node {
	void *target;
	struct st_node *next;
};

typedef struct st_list List;

struct st_list {
	struct st_node *head;
	int length;
};

List *list_create();

struct st_node *list_get(struct st_list *list, int index);

struct st_node *list_search(struct st_list *list, int (*match)(void *));

void list_append(struct st_list *list, void *target);

void list_free(struct st_list *list);

#endif // TTETRIS_LIST_H
