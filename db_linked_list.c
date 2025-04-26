#include "db_linked_list.h"

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

    if (l->head == NULL) {
        l->head = node;
        l->tail = node;
    }

    l->tail->next = node;
    l->tail = node;

    l->count++;
}

void add_first(List *l, void *data) {

    if (l == NULL) {
        fprintf(stderr, "List cannot be null\n");
    }

    Node *node = malloc(sizeof(Node));
    node->data = data;
    
    node->next = l->head;
    l->head = node;

    l->count++;
}

void * get(List *l, int i) {

    if (l == NULL) {
        fprintf(stderr, "List cannot be null\n");
    }

    if (i > l->count) { return NULL; }

    Node *search = l->head;
    for (int j = 0; j <= i; j++) {
        if (search->next == NULL) {
            fprintf(stderr, "Error, count of linked list is wrong\n");

        }
        search = search->next;
    }

    return search->data;
}

