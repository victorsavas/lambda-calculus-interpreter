#include <string.h>

#include "stack.h"
#include "variable.h"

#define STACK_CAP 16

struct Stack {
        struct Variable *array;
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

size_t stack_push(Stack *stack, struct Variable variable)
{
        if (stack == NULL)
                return 0;

        if (stack->top + 1 == stack->capacity) {
                struct Variable *array = realloc(stack->array, stack->capacity << 1);

                if (array == NULL)
                        return 0;
                
                stack->array = array;
                stack->capacity <<= 1;
        }

        stack->array[stack->top++] = variable;

        return stack->top;
}

struct Variable stack_pop(Stack *stack)
{
        struct Variable error = {
                .letter = '\0',
                .subscript = -1
        };

        if (stack == NULL)
                return error;

        if (stack->top == 0)
                return error;

        return stack->array[--stack->top];
}

struct Variable stack_peek(Stack *stack)
{
        struct Variable error = {
                .letter = '\0',
                .subscript = -1
        };
        
        if (stack == NULL)
                return error;

        if (stack->top == 0)
                return error;

        return stack->array[stack->top];
}

bool stack_empty(Stack *stack)
{
        if (stack == NULL)
                return false;

        return stack->top == 0;
}

bool stack_search(Stack *stack, struct Variable variable)
{
        if (stack == NULL)
                return false;

        for (size_t i = stack->top; i-- > 0;) {
                struct Variable var = stack->array[i];

                if (variable_compare(var, variable))
                        return true;
        }

        return false;
}