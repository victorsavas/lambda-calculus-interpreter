#pragma once

#include <stdbool.h>
#include <stdlib.h>

#include "variable.h"

typedef struct Stack Stack;

Stack *stack_init();
void stack_free(Stack *stack);

size_t stack_push(Stack *stack, struct Variable variable);
struct Variable stack_pop(Stack *stack);
struct Variable stack_peek(Stack *stack);

bool stack_empty(Stack *stack);
bool stack_search(Stack *stack, struct Variable variable);