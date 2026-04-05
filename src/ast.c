#include <stdio.h>

#include "ast.h"
#include "stack.h"

void lambda_free(Lambda *lambda)
{
        if (lambda == NULL)
                return;

        Stack *stack = stack_init();

        if (stack == NULL) {
                fprintf(stderr, "Fatal error. Unable to initialize stack.\n");
                exit(EXIT_FAILURE);
        }

        Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        free(top->entry);
                        stack_push(stack, top->expression);
                        break;
                
                case LAMBDA_SHORTCUT:
                        free(top->shortcut);
                        break;

                case LAMBDA_VARIABLE:
                        break;

                case LAMBDA_ABSTRACTION:
                        stack_push(stack, top->body);
                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->right);
                        stack_push(stack, top->left);
                        break;
                
                case LAMBDA_NUMERAL:
                        break;

                // case LAMBDA_INDIRECTION:
                //         stack_push(stack, top->indirection);
                //         break;
                }

                free(top);
                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);
}