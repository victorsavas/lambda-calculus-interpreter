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

void lambda_print(const Lambda *lambda, const Lambda *redex)
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

        stack_push(operator_stack, operator_table[OPERATOR_EMPTY]);

        const Lambda *top = lambda;
        const char *operator_top = NULL;

        while (top != NULL) {
                if (redex != NULL) {
                        if (top == redex->right) {
                                stack_push(operator_stack, operator_table[OPERATOR_RESET]);
                                printf(ANSI_GREEN);
                        }

                        if (top == redex->left) {
                                stack_push(operator_stack, operator_table[OPERATOR_RESET]);
                                printf(ANSI_RED);
                        }
                }

                switch (top->type) {
                case LAMBDA_ENTRY:
                        printf("%s", lambda->shortcut);
                        stack_push(lambda_stack, top->expression);
                        stack_push(operator_stack, operator_table[OPERATOR_EQUALS]);
                        break;

                case LAMBDA_SHORTCUT:
                        printf("%s", lambda->shortcut);
                        stack_push(operator_stack, operator_table[OPERATOR_SPACE]);
                        break;

                case LAMBDA_VARIABLE:
                        struct Variable variable = top->variable;
                        
                        variable_print(variable);

                        break;
                
                case LAMBDA_ABSTRACTION:
                        struct Variable bind = top->bind;
                        
                        printf("%s", operator_table[OPERATOR_LAMBDA]);

                        variable_print(bind);

                        stack_push(lambda_stack, top->body);
                        stack_push(operator_stack, operator_table[OPERATOR_DOT]);

                        break;

                case LAMBDA_APPLICATION:
                        Lambda *left = top->left;
                        Lambda *right = top->right;

                        stack_push(lambda_stack, right);
                        stack_push(lambda_stack, left);

                        bool left_parenthesis = left->type == LAMBDA_ABSTRACTION;
                        bool right_parenthesis = right->type != LAMBDA_VARIABLE;

                        bool color = top == redex;

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

                top = stack_pop(lambda_stack);

                do {
                        operator_top = stack_pop(operator_stack);

                        if (operator_top != NULL)
                                printf("%s", operator_top);
                } while (
                        operator_top == operator_table[OPERATOR_RP]
                     || operator_top == operator_table[OPERATOR_RESET]);
        }

        stack_free(lambda_stack);
        stack_free(operator_stack);
}