#include "list.h"

#include <stdio.h>

struct list_node *list_node_create(void *element);
void list_node_free(struct list_node *node);

struct list_node *list_get_node_at(struct list *list, int index);

void list_add_node_after(struct list *list, struct list_node *previous_node, struct list_node *new_node);
void *list_remove_node(struct list *list, struct list_node *target_node);

struct list_iterator *list_iterator_create(struct list *list, struct list_node *current_node);

struct list *list_create() {
	struct list *list;

	list = (struct list *)malloc(sizeof(struct list));
	list->head = list_node_create(NULL);
	list->tail = list_node_create(NULL);
	list->head->next = list->tail;
	list->tail->previous = list->head;
	list->length = 0;

	return list;
}

void list_free(struct list *list) {
	list_clear(list);

	list_node_free(list->head);
	list_node_free(list->tail);
	
	free(list);
}

struct list *list_clone(struct list *list) {
	struct list *clone_list;
	struct list_iterator *iterator;

	clone_list = list_create();
	iterator = list_first_iterator(list);
	while (list_iterator_has_next(iterator)) {
		list_add_last(clone_list, list_iterator_next(iterator));
	}
	list_iterator_free(iterator);

	return clone_list;
}

struct list_node *list_node_create(void *element) {
	struct list_node *new_node;
	
	new_node = (struct list_node *)malloc(sizeof(struct list_node));
	new_node->previous = NULL;
	new_node->element = element;
	new_node->next = NULL;

	return new_node;
}

void list_node_free(struct list_node *node) {
	free(node);
    
}

struct list_node *list_get_node_at(struct list *list, int index) {
	struct list_node *node;

	assert((0 <= index) && (index < list->length));
	
	node = list->head;
	for (int i = 0; i <= index; i += 1) {
		node = node->next;
	}

	return node;
}

void *list_get(struct list *list, int index) {
	return list_get_node_at(list, index)->element;
}

void list_set(struct list *list, int index, void *element) {
	struct list_node *node = list_get_node_at(list, index);
	node->element = element;
}

void list_add_node_after(struct list *list, struct list_node *previous_node, struct list_node *new_node) {
	new_node->previous = previous_node;
	new_node->next = previous_node->next;

	previous_node->next->previous = new_node;
	previous_node->next = new_node;	

	list->length += 1;
}

void list_add(struct list *list, int index, void *element) {
	assert((0 <= index) && (index <= list->length));
	
	if (index == 0) {
		list_add_first(list, element);
	} else if (index == list->length) {
		list_add_last(list, element);
	} else {
		struct list_node *previous_node;
		struct list_node *new_node;

		previous_node = list_get_node_at(list, index - 1);
		new_node = list_node_create(element);
		list_add_node_after(list, previous_node, new_node);
	}
}

void list_add_first(struct list *list, void *element) {
	list_add_node_after(list, list->head, list_node_create(element));
}

void list_add_last(struct list *list, void *element) {
	list_add_node_after(list, list->tail->previous, list_node_create(element));
}

void *list_remove_node(struct list *list, struct list_node *target_node) {
	void *element = target_node->element;

	target_node->previous->next = target_node->next;
	target_node->next->previous = target_node->previous;

	list_node_free(target_node);
	list->length -= 1;

	return element;
}

void * list_remove_at(struct list *list, int index) {
	assert((0 <= index) && (index < list->length));

	return list_remove_node(list, list_get_node_at(list, index));
}

void * list_remove_first(struct list *list) {
	assert(list->length > 0);

	return list_remove_node(list, list->head->next);
}

void *list_remove_last(struct list *list) {
	assert(list->length > 0);

	return list_remove_node(list, list->tail->previous);
}

struct list_iterator *list_first_iterator(struct list *list) {
	return list_iterator_create(list, list->head);
}

struct list_iterator *list_last_iterator(struct list *list) {
	return list_iterator_create(list, list->tail);
}

void list_clear(struct list *list) {
	struct list_iterator *iterator;

	iterator = list_first_iterator(list);
	while (list_iterator_has_next(iterator)) {
		list_iterator_next(iterator);
		list_iterator_remove(iterator);
	}
	list_iterator_free(iterator);
}

int list_length(struct list *list) {
	return list->length;
}

void **list_to_array(struct list *list) {
    void **array = (void **)malloc(sizeof(void *) * list->length);
    void **element = array;
    
    struct list_iterator *iterator = list_first_iterator(list);
    while (list_iterator_has_next(iterator)) {
        *element = list_iterator_next(iterator);
        element++;
    }
    
    return array;
}

struct list_iterator *list_iterator_create(struct list *list, struct list_node *current_node) {
	struct list_iterator *iterator;

	iterator = (struct list_iterator *)malloc(sizeof(struct list_iterator));
	iterator->list = list;
	iterator->current = current_node;
	iterator->previous = current_node->previous;
	iterator->next = current_node->next;

	return iterator;
}

void list_iterator_free(struct list_iterator *iterator) {
	free(iterator);
}

int list_iterator_has_next(struct list_iterator *iterator) {
	return (iterator->next->next != NULL);
}

int list_iterator_has_previous(struct list_iterator *iterator) {
	return (iterator->previous->previous != NULL);
}

void *list_iterator_next(struct list_iterator *iterator) {
	assert(list_iterator_has_next(iterator));

	iterator->current = iterator->next;
	iterator->previous = iterator->current->previous;
	iterator->next = iterator->current->next;

	return iterator->current->element;
}

void *list_iterator_previous(struct list_iterator *iterator) {
	assert(list_iterator_has_previous(iterator));
	
	iterator->current = iterator->previous;
	iterator->previous = iterator->current->previous;
	iterator->next = iterator->current->next;

	return iterator->current->element;
}

void *list_iterator_get(struct list_iterator *iterator) {
	assert(iterator->current != NULL); /* because of removal */

	return iterator->current->element;
}

void *list_iterator_get_next(struct list_iterator *iterator) {
    assert(list_iterator_has_next(iterator));
    
    return iterator->next->element;
}

void *list_iterator_get_previous(struct list_iterator *iterator) {
    assert(list_iterator_has_previous(iterator));
    
    return iterator->previous->element;
}

void list_iterator_set(struct list_iterator *iterator, void *element) {
	assert(iterator->current != NULL); /* because of removal */

	iterator->current->element = element;
}

void *list_iterator_remove(struct list_iterator *iterator) {
	void *element = iterator->current->element;
	
	list_remove_node(iterator->list, iterator->current);
	iterator->current = NULL;

	return element;
}