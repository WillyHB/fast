#include "../include/db_linked_list.h"

void *init_list() {
    List *l = malloc(sizeof(List));
    l->head = NULL;
    l->tail = NULL;
    l->count = 0;

    return l;
}

void add_last(List *l, void *data) {

    if (l == NULL) {
        fprintf(stderr, "List cannot be null");
    }

    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    if (l->count <= 0) {
        l->head = node;
    } else {
        // Wait a moment.
        node->prev = l->tail;
        node->prev->next = node;
    }

    l->tail = node;

    l->count++;
}

void add_first(List *l, void *data) {

    if (l == NULL) {
        fprintf(stderr, "List cannot be null\n");
    }

    Node *node = malloc(sizeof(Node));
    node->data = data;
    node->next = NULL;
    node->prev = NULL;

    // So first element is the tail and head
    if (l->count <= 0) {
        l->tail = node;
    } else {
        node->next = l->head;
        node->next->prev = node;
    }
    
    l->head = node;

    l->count++;
}

void * get(List *l, int i) {

    if (l == NULL) { fprintf(stderr, "List cannot be null\n"); }
    if (i > l->count || i < 0) { return NULL; }

    Node *search;

    if (i > l->count/2) {
        search = l->tail;
        for (int j = l->count-1; j > i; j--) {
            if (search->prev == NULL) {
                fputs("Error, count of linked list is wrong", stderr);
            }
            search = search->prev;
        }

    } else {
        search = l->head;
        for (int j = 0; j < i; j++) {
            if (search->next == NULL) {
                fputs("Error, count of linked list is wrong", stderr);
            }
            search = search->next;
        }

    }

    return search->data;
}

void *get_last(List *list) {
    if (list == NULL) { return NULL; }
    if (list->count <= 0) { return NULL; }
    return list->tail->data;
}

void *get_first(List *list) {
    if (list == NULL) { return NULL; }
    if (list->count <= 0) { return NULL; }
    return list->head->data;
}

void *remove_first(List *list) {
    if (list == NULL) { return NULL; }
    if (list->count <= 0) { return NULL; }

    Node *n = list->head;
    void *data = n->data;
    free (n);

    list->head = list->head->next;
    list->count--;
    
    return data;
}

void *remove_last(List *list) {
    if (list == NULL) { return NULL; }
    if (list->count <= 0) { return NULL; }

    Node *n = list->tail;
    void *data = n->data;
    free(n);
    list->tail = list->tail->prev;

    list->count--;

    return data;
}

void *remove_at(List *list, int index) {
    if (list == NULL) { return NULL; }
    if (index >= list->count) { return NULL; }

    Node *search = list->head;

    // Loop until we get the node right before the one we wish to delete
    for (int i = 0; i < index-1; i++) {
        search = search->next;
    }

    // Store the one we want to delete
    Node *n = search->next;
    void *data = n->data;
    free(n);

    // replace next node with node 1 over
    search->next = search->next->next;

    // free deleted node
    return data;
}

void set(List *list, int index, void *data) {

}

void set_last(List *list, void *data) {
    list->tail->data = data;
}

void set_first(List *list, void *data) {
    list->head->data = data;
}



