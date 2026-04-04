#include <stdlib.h>
#include <string.h>

#include "duplicate.h"
#include "stack.h"

Lambda *lambda_duplicate(const Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Stack *src_stack = stack_init();
        Stack *dest_stack = stack_init();

        Lambda *dup = calloc(1, sizeof(*dup));

        if (src_stack == NULL || dest_stack == NULL || dup == NULL) {
                stack_free(src_stack);
                stack_free(dest_stack);
                free(dup);
                return NULL;
        }

        Lambda *src_top = (Lambda *)lambda;
        Lambda *dest_top = dup;

        while (src_top != NULL) {
                Lambda *left;
                Lambda *right;

                dest_top->type = src_top->type;

                switch (src_top->type) {
                case LAMBDA_ENTRY:
                        right = calloc(1, sizeof(*right));

                        dest_top->entry = my_strdup(src_top->entry);
                        dest_top->term = right;

                        stack_push(src_stack, src_top->term);
                        stack_push(dest_stack, right);

                        break;

                case LAMBDA_SHORTCUT:
                        dest_top->shortcut = my_strdup(src_top->shortcut);
                        break;

                case LAMBDA_VARIABLE:
                        dest_top->variable = src_top->variable;
                        break;

                case LAMBDA_ABSTRACTION:
                        right = calloc(1, sizeof(*right));

                        dest_top->variable = src_top->variable;
                        dest_top->body = right;

                        stack_push(src_stack, src_top->body);
                        stack_push(dest_stack, right);

                        break;

                case LAMBDA_APPLICATION:
                        right = malloc(sizeof(*right));
                        left = malloc(sizeof(*left));

                        dest_top->right = right;
                        dest_top->left = left;

                        stack_push(src_stack, src_top->right);
                        stack_push(src_stack, src_top->left);

                        stack_push(dest_stack, right);
                        stack_push(dest_stack, left);

                        break;
                
                case LAMBDA_NUMERAL:
                        dest_top->numeral = src_top->numeral;
                        break;
                }

                src_top = (Lambda *)stack_pop(src_stack);
                dest_top = (Lambda *)stack_pop(dest_stack);
        }

        stack_free(src_stack);
        stack_free(dest_stack);

        return dup;
}

char *my_strdup(char *str)
{
        if (str == NULL)
                return NULL;

        size_t length = strlen(str) + 1;
        char *duplicate = malloc(length);

        if (duplicate == NULL)
                return NULL;

        strcpy(duplicate, str);

        return duplicate;
}