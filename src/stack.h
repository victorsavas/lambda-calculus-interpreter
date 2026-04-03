#pragma once

#include <stdbool.h>
#include <stdlib.h>

typedef struct Stack Stack;

Stack *stack_init();
void stack_free(Stack *stack);

const void *stack_push(Stack *stack, const void *address);
const void *stack_pop(Stack *stack);
const void *stack_peek(Stack *stack);
const void *stack_search(Stack *stack, const void *address,
                         bool compare(const void *left, const void *right));