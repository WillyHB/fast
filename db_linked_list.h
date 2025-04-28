#include <stdlib.h>
#include <stdio.h>

#ifndef dblist
#define dblist
typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;
} Node;

typedef struct List {
    struct Node *head;
    struct Node *tail;
    int count;
} List;

void *init_list     ();

void *get           (List*, int);
void *get_last      (List *);
void *get_first     (List *);

void add_first      (List*, void *);
void add_last       (List*, void *);

void *remove_at     (List*, int);
void *remove_first  (List*);
void *remove_last   (List*);

void set            (List*, int, void*);
void set_last       (List*, void*);
void set_first      (List*,void*);

#endif
