#include <stdbool.h>
#include <stdio.h>

#include "ansi_escapes.h"
#include "printing.h"
#include "stack.h"

enum Operator {
        OPERATOR_EMPTY          = 0,
        OPERATOR_LP             = 1,
        OPERATOR_RP             = 2,
        OPERATOR_LAMBDA         = 3,
        OPERATOR_DOT            = 4,
        OPERATOR_SPACE          = 5,
        OPERATOR_EQUALS         = 6,
        OPERATOR_GREEN          = 7,
        OPERATOR_RED            = 8,
        OPERATOR_RESET          = 9
};

const char *operator_table[] = {
        "",
        "(",
        ")",
        "λ",
        ".",
        " ",
        "=",
        ANSI_GREEN,
        ANSI_RED,
        ANSI_RESET
};

void lambda_print(const Lambda *lambda, const Lambda *highlight)
{
        if (lambda == NULL)
                return;

        Stack *lambda_stack = stack_init();
        Stack *operator_stack = stack_init();

        if (lambda_stack == NULL || operator_stack == NULL) {
                stack_free(lambda_stack);
                stack_free(operator_stack);
                return;
        }

        stack_push(lambda_stack, lambda);
        stack_push(operator_stack, operator_table[OPERATOR_EMPTY]);

        const Lambda *lambda_top = stack_pop(lambda_stack);
        const char *operator_top = NULL;

        while (lambda_top != NULL) {
                switch (lambda_top->type) {
                case LAMBDA_BIND:
                        printf("%s", lambda->bind.shortcut);
                        stack_push(lambda_stack, lambda_top->bind.term);
                        stack_push(operator_stack, operator_table[OPERATOR_EQUALS]);
                        break;

                case LAMBDA_SHORTCUT:
                        printf("%s", lambda->shortcut);
                        stack_push(operator_stack, operator_table[OPERATOR_SPACE]);
                        break;

                case LAMBDA_VARIABLE:
                        struct Variable variable = lambda_top->variable;
                        
                        variable_print(variable);

                        break;
                
                case LAMBDA_ABSTRACTION:
                        struct Variable binding = lambda_top->abstraction.binding;
                        
                        printf(operator_table[OPERATOR_LAMBDA]);

                        variable_print(binding);

                        stack_push(lambda_stack, lambda_top->abstraction.body);
                        stack_push(operator_stack, operator_table[OPERATOR_DOT]);

                        break;

                case LAMBDA_APPLICATION:
                        Lambda *left = lambda_top->application.left;
                        Lambda *right = lambda_top->application.right;

                        stack_push(lambda_stack, right);
                        stack_push(lambda_stack, left);

                        bool left_parenthesis = left->type == LAMBDA_ABSTRACTION;
                        bool right_parenthesis = right->type != LAMBDA_VARIABLE;

                        bool color = lambda_top == highlight;

                        if (right_parenthesis) {
                                stack_push(operator_stack, operator_table[OPERATOR_RP]);
                                stack_push(operator_stack, operator_table[OPERATOR_LP]);
                        } else {
                                stack_push(operator_stack, operator_table[OPERATOR_EMPTY]);
                        }

                        if (left_parenthesis) {
                                stack_push(operator_stack, operator_table[OPERATOR_RP]);
                                stack_push(operator_stack, operator_table[OPERATOR_LP]);
                        } else {
                                stack_push(operator_stack, operator_table[OPERATOR_EMPTY]);
                        }
                }

                lambda_top = stack_pop(lambda_stack);

                do {
                        operator_top = stack_pop(operator_stack);

                        if (operator_top != NULL)
                                printf("%s", operator_top);
                } while (operator_top == operator_table[OPERATOR_RP]);
        }

        stack_free(lambda_stack);
        stack_free(operator_stack);
}