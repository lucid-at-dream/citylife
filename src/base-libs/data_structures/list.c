#include "list.h"

#include <stdlib.h>

list *list_new()
{
    list *l = (list *)calloc(1, sizeof(list));
    l->head = l->tail = NULL;
    return l;
}

void list_destroy(list *l)
{
    list_node *next = l->head;
    while (next != NULL)
    {
        list_node *tmp = next->next;
        free(next);
        next = tmp;
    }
    free(l);
}

void *list_get_first(list *l)
{
    return l->head->value;
}

void *list_get_last(list *l)
{
    return l->tail->value;
}

void *list_del_first(list *l)
{
    if (l->head == NULL)
    {
        return NULL;
    }

    list_node *tmp = l->head;
    l->head = l->head->next;

    if (l->head == NULL)
    {
        l->tail = NULL;
    }
    else
    {
        l->head->prev = NULL;
        if (l->head->next == NULL)
        {
            l->tail = l->head;
        }
    }

    void *value = tmp->value;
    free(tmp);
    l->size -= 1;

    return value;
}

void *list_del_last(list *l)
{
    if (l->tail == NULL)
    {
        return NULL;
    }

    list_node *tmp = l->tail;
    l->tail = l->tail->prev;

    if (l->tail == NULL)
    {
        l->head = NULL;
    }
    else
    {
        l->tail->next = NULL;
        if (l->tail->prev == NULL)
        {
            l->tail = l->head;
        }
    }

    void *value = tmp->value;
    free(tmp);
    l->size -= 1;

    return value;
}

// Private: alloc a new list node.
list_node *new_node_with_value(void *value)
{
    list_node *new_node = (list_node *)calloc(1, sizeof(list_node));
    new_node->value = value;
    new_node->next = new_node->prev = NULL;
    return new_node;
}

list_node *list_append(list *l, void *value)
{
    list_node *new_node = new_node_with_value(value);

    if (l->head == NULL)
    {
        // empty list scenario
        l->head = new_node;
        l->tail = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
    }
    else
    {
        new_node->next = NULL;
        new_node->prev = l->tail;
        l->tail->next = new_node;
        l->tail = new_node;
    }

    l->size += 1;
    return new_node;
}

list_node *list_prepend(list *l, void *value)
{
    list_node *new_node = new_node_with_value(value);

    // If the list is empty set head = tail
    if (l->head == NULL)
    {
        l->tail = new_node;
        l->head = new_node;
        new_node->prev = NULL;
        new_node->next = NULL;
    }
    else
    {
        new_node->prev = NULL;
        new_node->next = l->head;
        l->head->prev = new_node;
        l->head = new_node;
    }

    l->size += 1;
    return new_node;
}

char list_del_node(list *l, list_node *n)
{
    if (n->prev != NULL)
    {
        n->prev->next = n->next;
    }
    else
    {
        l->head = n->next; // n was the head of the list.
    }
    if (n->next != NULL)
    {
        n->next->prev = n->prev;
    }
    else
    {
        l->tail = n->prev; // n was the tail of the list.
    }
    free(n);
    l->size -= 1;
    return 1;
}

char list_del_element(list *l, void *value)
{
    list_node *tmp = l->head;
    while (tmp != NULL && tmp->value != value)
    {
        tmp = tmp->next;
    }

    if (tmp != NULL)
    {
        return list_del_node(l, tmp);
    }

    return 0; // Element not found;
}

char list_is_empty(list *l)
{
    return l->head == NULL;
}

void list_foreach(list *l, void (*action)(void **))
{
    list_node *tmp = l->head;
    while (tmp != NULL)
    {
        action(&(tmp->value));
        tmp = tmp->next;
    }
}
