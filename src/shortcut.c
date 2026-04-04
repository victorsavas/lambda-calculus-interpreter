#include <stdio.h>
#include <stdlib.h>

#include "ansi_escapes.h"
#include "duplicate.h"
#include "shortcut.h"
#include "stack.h"

static Lambda *generate_numeral(int integer);

bool replace_shortcuts(Lambda *lambda, HashTable *table)
{
        if (lambda == NULL)
                return false;

        Stack *stack = stack_init();

        if (stack == NULL)
                return false;

        Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        stack_push(stack, top->term);
                        break;

                case LAMBDA_SHORTCUT:
                        char *shortcut = top->shortcut;

                        Lambda *entry = hashtable_search(table, shortcut);

                        if (entry == NULL) {
                                printf(
                                        ANSI_RED
                                        "Error. Undefined entry \"%s\".\n"
                                        ANSI_RESET,
                                        shortcut
                                );
                                goto error;
                        }

                        Lambda *duplicate = lambda_duplicate(entry->term);

                        if (duplicate == NULL) {
                                printf(ANSI_RED "Error. Duplication fail.\n" ANSI_RESET);
                                goto error;
                        }

                        *top = *duplicate;

                        free(shortcut);
                        free(duplicate);                        

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
                        int integer = top->numeral;
                        Lambda *numeral = generate_numeral(integer);
                        
                        if (numeral == NULL) {
                                printf(
                                        ANSI_RED
                                        "Error. Failed to generate \"%d\" numeral.\n"
                                        ANSI_RESET,
                                        integer
                                );
                                goto error;
                        }

                        *top = *numeral;
                        free(numeral);
                        
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);
        return true;

        error:

        stack_free(stack);
        return false;
}

Lambda *generate_numeral(int integer)
{
        struct Variable var_f = {
                .letter = 'f',
                .subscript = -1
        };

        struct Variable var_x = {
                .letter = 'x',
                .subscript = -1
        };

        if (integer < 0)
                return NULL;

        Lambda *numeral = malloc(sizeof(*numeral));

        if (numeral == NULL)
                return NULL;

        numeral->type = LAMBDA_ABSTRACTION;
        numeral->variable = var_f;
        
        Lambda *inner_abstraction = malloc(sizeof(*inner_abstraction));
        
        numeral->body = inner_abstraction;

        if (inner_abstraction == NULL) {
                free(numeral);
                return NULL;
        }

        inner_abstraction->type = LAMBDA_ABSTRACTION;
        inner_abstraction->variable = var_x;

        Lambda *right;

        // Particular case without function application 0=\f.\x.x
        if (integer == 0) {
                right = malloc(sizeof(*right));

                inner_abstraction->body = right;

                if (right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                right->type = LAMBDA_VARIABLE;
                right->variable = var_x;

                return numeral;
        }

        // Generate a chain of applications of the form \f.\x.f(f(...(fx)...))
        Lambda *outer_application = malloc(sizeof(*outer_application));
        inner_abstraction->body = outer_application;

        if (outer_application == NULL){
                lambda_free(numeral);
                return NULL;
        }

        Lambda *application = outer_application;

        for (int k = 0; k < integer; k++) {
                application->type = LAMBDA_APPLICATION;

                Lambda *left = malloc(sizeof(*left));
                right = malloc(sizeof(*right));

                application->left = left;
                application->right = right;

                if (left == NULL || right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                left->type = LAMBDA_VARIABLE;
                left->variable = var_f;

                if (k + 1 == integer) {
                        right->type = LAMBDA_VARIABLE;
                        right->variable = var_x;

                        application->left = left;
                        application->right = right;
                } else {
                        application->left = left;
                        application->right = right;

                        application = right;
                }
        }

        return numeral;
}