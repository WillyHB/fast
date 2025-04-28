#include "stack.h"
#include <stdlib.h>

void free_stack(Stack *stack) {
    clear(stack);
    free(stack);
}

Stack * stack_init() {
    Stack *stack = malloc(sizeof(Stack));
    stack->head = NULL;
    return stack;
}

// Data is not copied
void push(Stack *stack, void *data) {
    if (stack == NULL) {
        fputs("Inputted stack is null", stderr);
        return;
    }

    // Initialises the node
    Node *node = malloc(sizeof(Node));
    node->next = NULL;
    node->prev = NULL;
    node->data = data;

    // If there is a head
    if (stack->head != NULL) {
        // Set the nodes next to the current head
        node->next = stack->head;
        // Set the current head's previous to the node
        stack->head->prev = node;
    }

    // Makes the node the current head of the stack - for quicker pop
    stack->head = node;
}

void * peek(Stack *stack) {
    if (stack == NULL) {
        fputs("Inputted stack is null", stderr);
        return NULL;
    }

    if (stack->head == NULL) {
        return NULL;
    }

    return stack->head->data;
}

void * pop(Stack *stack) {
    if (stack == NULL) {
        fputs("Inputted stack is null", stderr);
        return NULL;
    }

    if (stack->head == NULL) {
        return NULL;
    }

    void *data = stack->head->data;

    stack->head = stack->head->next;

    stack->head->prev = NULL;

    return data;
}

void clear_loop(Node *node) {
    if (node != NULL) {
        free(node->data);
        clear_loop(node->next);
        node = NULL;
    }
}

// Frees all data
void clear(Stack *stack) {
    if (stack == NULL) {
        fputs("Inputted stack is null", stderr);
        return;
    }

    Node *search = stack->head;

    clear_loop(search);
}

