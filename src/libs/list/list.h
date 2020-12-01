#pragma once

#define LAMBDA(c_) ({ c_ _;})

/**
 * Represents a node in a linked list.
 */
typedef struct list_node_t {
    struct list_node_t *next;
    struct list_node_t *prev;
    void *value;
} list_node;

/**
 * Linked list base structure.
 */
typedef struct list_t {
    list_node *head;
    list_node *tail;
} list;

/**
 * Allocates a new, empty, list.
 * complexity: O(1)
 */
list *list_new();

/**
 * Destroys a list and all meta (list_node struct) elements in it.
 * complexity: O(N)
 */
void list_destroy(list *);

/**
 * Retrieves the first element in the list.
 * complexity: O(1)
 */
void *list_get_first(list *);

/**
 * Retrieves the last element in the list.
 * complexity: O(1)
 */
void *list_get_last(list *);

/**
 * Deletess the first element in the list, returning its value.
 * complexity: O(1)
 */
void *list_del_first(list *);

/**
 * Deletes the last element in the list, returning its value.
 * complexity: O(1)
 */
void *list_del_last(list *);

/**
 * Append an element to the end of the list with the given value.
 * complexity: O(1)
 */
void list_append(list *, void *);

/**
 * Prepend an element to the begining of the list with the given value.
 * complexity: O(1)
 */
void list_prepend(list *, void *);

/**
 * Delete the first element from the list with the given value.
 * Returns 0 if the element is deleted or 1 if it is not found.
 * complexity: O(N)
 */
char list_del_element(list *, void *);

/**
 * Check if the list is empty.
 * Returns 1 if is empty and 0 otherwise.
 * complexity: O(1)
 */
char list_is_empty(list *);

/**
 * Applies the given action to each of the elements.
 * complexity: O(N)
 */
void foreach(list *l, void (*action)(void **));
