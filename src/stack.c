#include <string.h>
#include "stack.h"

struct Stack {
        char **array;
        size_t top;
        size_t capacity;
};

Stack *stack_init(size_t capacity)
{
        Stack *stack = malloc(sizeof(*stack));

        if (stack == NULL)
                return NULL;

        stack->array = malloc(sizeof(*stack->array) * capacity);

        if (stack->array == NULL) {
                free(stack);
                return NULL;
        }

        stack->top = 0;
        stack->capacity = capacity;

        return stack;
}

void stack_free(Stack *stack)
{
        if (stack == NULL)
                return;

        free(stack->array);
        free(stack);
}

size_t stack_push(Stack *stack, char *str)
{
        if (stack == NULL || str == NULL)
                return 0;

        if (stack->top + 1 == stack->capacity) {
                char **array = realloc(stack->array, stack->capacity << 1);

                if (array == NULL)
                        return 0;
                
                stack->array = array;
                stack->capacity <<= 1;
        }

        stack->array[stack->top++] = str;

        return stack->top;
}

char *stack_pop(Stack *stack)
{
        if (stack == NULL)
                return NULL;

        if (stack->top == 0)
                return NULL;

        return stack->array[--stack->top];
}

char *stack_peek(Stack *stack)
{
        if (stack == NULL)
                return NULL;

        if (stack->top == 0)
                return NULL;

        return stack->array[stack->top];
}

char *stack_search(Stack *stack, char *name)
{
        if (stack == NULL || name == NULL)
                return NULL;

        for (size_t i = stack->top; i-- > 0;) {
                char *str = stack->array[i];

                if (strcmp(str, name) == 0)
                        return str;
        }

        return NULL;
}