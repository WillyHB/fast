#include <stdlib.h>
#include <stdio.h>

#ifndef dblist
#define dblist
typedef struct Node {
    void *data;
    struct Node *next;
} Node;

typedef struct List {
    struct Node *head;
    struct Node *tail;
    int count;
} List;

void *init_list();
void *get(List*, int);
void add_first(List*, void *data);
void add_last(List*, void *data);
void remove_first(List*, void *data);
void remove_last(List*, void *data);
void remove_at(List*, int);

#endif
