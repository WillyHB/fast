#include <stdio.h>

#ifndef stackh
#define stackh

typedef struct Node {
    void *data;
    struct Node *next;
    struct Node *prev;

} Node;

typedef struct Stack {
    struct Node *head;

} Stack;

Stack *create_stack();
void push(Stack*, void *data);
void* pop(Stack*);
void* peek(Stack*);
void clear(Stack*);

#endif

