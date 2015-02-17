#ifndef hjh_list_h
#define hjh_list_h

#include <stdlib.h>
#include <assert.h>

struct list_node {
	struct list_node *previous;
	void *element;
	struct list_node *next;
};

struct list {
	struct list_node *head;
	struct list_node *tail;

	int length;
};

struct list_iterator {
	struct list *list;

	struct list_node *previous;
	struct list_node *current;
	struct list_node *next;
};

struct list *list_create();
void list_free(struct list *list);

struct list *list_clone(struct list *list);

void *list_get(struct list *list, int index);
void list_set(struct list *list, int index, void *element);

void list_add(struct list *list, int index, void *element);
void list_add_first(struct list *list, void *element);
void list_add_last(struct list *list, void *element);

void *list_remove_at(struct list *list, int index);
void *list_remove_first(struct list *list);
void *list_remove_last(struct list *list);

#define list_push(list, element) list_add_last((list), (element))
#define list_pop(list) list_remove_last((list))

#define list_shift(list, element) list_add_first((list), (element))
#define list_unshift(list) list_remove_first((list))

struct list_iterator *list_first_iterator(struct list *list);
struct list_iterator *list_last_iterator(struct list *list);

void list_clear(struct list *list);
int list_length(struct list *list);

void **list_to_array(struct list *list);

void list_iterator_free(struct list_iterator *iterator);

int list_iterator_has_next(struct list_iterator *iterator);
int list_iterator_has_previous(struct list_iterator *iterator);

void *list_iterator_next(struct list_iterator *iterator);
void *list_iterator_previous(struct list_iterator *iterator);

void *list_iterator_get(struct list_iterator *iterator);
void *list_iterator_get_next(struct list_iterator *iterator);
void *list_iterator_get_previous(struct list_iterator *iterator);
void list_iterator_set(struct list_iterator *iterator, void * element);

void *list_iterator_remove(struct list_iterator *iterator);

#endif