#include <stdbool.h>
#include <stdio.h>

#include "printing.h"

void lambda_print(Lambda *lambda)
{
        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_BIND:
                printf("%s=", lambda->bind.shortcut);
                lambda_print(lambda->bind.term);
                break;
        
        case LAMBDA_SHORTCUT:
                printf("%s", lambda->shortcut);
                break;
        
        case LAMBDA_VARIABLE:
                if (lambda->variable.subscript < 0)
                        printf("%c", lambda->variable.letter);
                else
                        printf("%c%d",
                                lambda->variable.letter,
                                lambda->variable.subscript);

                break;

        case LAMBDA_ABSTRACTION:
                if (lambda->abstraction.binding.subscript < 0)
                        printf("λ%c.", lambda->abstraction.binding.letter);
                else
                        printf("λ%c%d.",
                                lambda->abstraction.binding.letter,
                                lambda->abstraction.binding.subscript);
                
                lambda_print(lambda->abstraction.body);
                
                break;

        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                bool left_parenthesis = left->type == LAMBDA_ABSTRACTION;
                bool right_parenthesis = right->type != LAMBDA_VARIABLE;

                if (left_parenthesis)
                        printf("(");

                lambda_print(lambda->application.left);

                if (left_parenthesis)
                        printf(")");

                if (right_parenthesis)
                        printf("(");

                lambda_print(lambda->application.right);

                if (right_parenthesis)
                        printf(")");

                break;
        }
}