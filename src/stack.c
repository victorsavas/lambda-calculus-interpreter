#include <string.h>

#include "stack.h"

#define STACK_CAP 32

struct Stack {
        const void **array;
        size_t top;
        size_t capacity;
};

Stack *stack_init()
{
        Stack *stack = malloc(sizeof(*stack));

        if (stack == NULL)
                return NULL;

        stack->array = malloc(sizeof(*stack->array) * STACK_CAP);

        if (stack->array == NULL) {
                free(stack);
                return NULL;
        }

        stack->top = 0;
        stack->capacity = STACK_CAP;

        return stack;
}

void stack_free(Stack *stack)
{
        if (stack == NULL)
                return;

        free(stack->array);
        free(stack);
}

const void *stack_push(Stack *stack, const void *address)
{
        if (stack == NULL)
                return NULL;

        if (stack->top + 1 == stack->capacity) {
                const void **array = realloc(stack->array, sizeof(*array) * (stack->capacity << 1));

                if (array == NULL)
                        return NULL;
                
                stack->array = array;
                stack->capacity <<= 1;
        }

        stack->array[stack->top++] = address;

        return address;
}

const void *stack_pop(Stack *stack)
{
        if (stack == NULL)
                return NULL;

        if (stack->top == 0)
                return NULL;

        return stack->array[--stack->top];
}

const void *stack_peek(Stack *stack)
{
        if (stack == NULL)
                return NULL;

        if (stack->top == 0)
                return NULL;

        return stack->array[stack->top];
}

const void *stack_search(Stack *stack, const void *address,
                         bool compare(const void *left, const void *right))
{
        if (stack == NULL)
                return NULL;

        for (size_t i = 0; i < stack->top; i++) {
                const void *entry = stack->array[i];

                if (compare(address, entry))
                        return entry;
        }

        return NULL;
}