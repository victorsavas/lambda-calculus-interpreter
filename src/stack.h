#pragma once

#include <stdlib.h>

typedef struct Stack Stack;

Stack *stack_init(size_t capacity);
void stack_free(Stack *stack);

size_t stack_push(Stack *stack, char *str);
char *stack_pop(Stack *stack);
char *stack_peek(Stack *stack);

char *stack_search(Stack *stack, char *name);