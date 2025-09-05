#include <stdbool.h>
#include <stdio.h>

#include "printing.h"

void lambda_print(Lambda *lambda)
{
        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_BIND:
                printf("%s = ", lambda->bind.name);
                lambda_print(lambda->bind.term);
                break;
        
        case LAMBDA_VARIABLE:
                printf("%s", lambda->variable);
                break;

        case LAMBDA_ABSTRACTION:
                printf("λ%s.", lambda->abstraction.binding);
                lambda_print(lambda->abstraction.body);
                break;

        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                bool left_parenthesis = left->type == LAMBDA_ABSTRACTION;
                bool right_parenthesis = right->type != LAMBDA_VARIABLE;

                bool parenthesis_space = 
                        right->type == LAMBDA_VARIABLE
                     || (left->type == LAMBDA_VARIABLE && !right_parenthesis);

                if (left_parenthesis)
                        printf("(");

                lambda_print(lambda->application.left);

                if (left_parenthesis)
                        printf(")");

                if (parenthesis_space)
                        printf(" ");

                if (right_parenthesis)
                        printf("(");

                lambda_print(lambda->application.right);

                if (right_parenthesis)
                        printf(")");

                break;
        }
}