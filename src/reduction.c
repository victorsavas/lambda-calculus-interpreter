#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "alpha_rename.h"
#include "ansi_escapes.h"
#include "duplicate.h"
#include "printing.h"
#include "reduction.h"
#include "stack.h"
#include "variable.h"
#include "variable_capture.h"

#define LONG_CYCLE 10000

static bool is_redex(Lambda *lambda);

static Lambda *get_redex_normal(Lambda *lambda);
// static Lambda *get_redex_applicative(Lambda *lambda);
// static Lambda *get_redex_cb_name(Lambda *lambda);
// static Lambda *get_redex_cb_value(Lambda *lambda);

static void beta_reduction(Lambda *redex);

bool lambda_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        Lambda *redex = get_redex_normal(lambda);

        return redex == NULL;
}

Lambda *lambda_reduce(Lambda *lambda, struct Mode mode)
{
        if (lambda == NULL)
                return NULL;

        bool normal_form;

        if (!mode.reduction_enabled) {
                lambda_print(lambda, NULL);

                normal_form = lambda_normal(lambda);

                if (normal_form)
                        printf(ANSI_BLUE " (Normal form.)\n" ANSI_RESET);
                else
                        printf("\n");

                return lambda;
        }

        normal_form = false;

        unsigned int i;

        for (i = 0; i < mode.depth; i++) {
                Lambda *redex = NULL;

                /*
                switch (mode.strat) {
                case STRAT_NORMAL:
                        redex = get_redex_normal(lambda);
                        break;

                case STRAT_APPLICATIVE:
                        redex = get_redex_applicative(lambda);
                        break;

                case STRAT_CALL_BY_NAME:
                        redex = get_redex_cb_name(lambda);
                        break;

                case STRAT_CALL_BY_VALUE:
                        redex = get_redex_cb_value(lambda);
                        break;
                }

                */

                redex = get_redex_normal(lambda);

                if (redex == NULL) {
                        normal_form = true;
                        break;
                }

                if (mode.verbose) {
                        printf(ANSI_BLUE "%-5u " ANSI_RESET, i + 1);
                        lambda_print(lambda, redex);
                        printf("\n");
                } else if ((i + 1) % LONG_CYCLE == 0) {
                        printf(".\n");
                }

                Lambda *capture = variable_capture(redex);

                if (capture == NULL)
                        beta_reduction(redex);
                else
                        alpha_rename(capture, redex);
        }

        lambda_print(lambda, NULL);

        if (normal_form)
                printf(ANSI_BLUE " (Normal form reached after %d steps.)\n" ANSI_RESET, i);
        else
                printf(ANSI_BLUE " (Normal form not reached.)\n" ANSI_RESET);

        return lambda;
}

void beta_reduction(Lambda *redex)
{
        if (redex == NULL)
                return;

        if (!is_redex(redex))
                return;

        Stack *stack = stack_init();

        if (stack == NULL)
                return;

        Lambda *left = redex->left;
        Lambda *argument = redex->right;

        Lambda *body = left->body;

        struct Variable bound_var = left->variable;

        free(left);

        Lambda *top = body;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                case LAMBDA_SHORTCUT:
                        // illegal
                        stack_free(stack);
                        return;

                case LAMBDA_VARIABLE:
                        struct Variable var = top->variable;

                        if (!variable_compare(var, bound_var))
                                break;

                        Lambda *dup = lambda_duplicate(argument);

                        *top = *dup;
                        free(dup);

                        break;
                        
                case LAMBDA_ABSTRACTION:
                        var = top->variable;

                        if (variable_compare(var, bound_var))
                                break;

                        Lambda *body = top->body;
                        stack_push(stack, body);

                        break;

                case LAMBDA_APPLICATION:
                        Lambda *right = top->right;
                        Lambda *left = top->left;

                        stack_push(stack, right);
                        stack_push(stack, left);

                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        lambda_free(argument);
        *redex = *body;
        free(body);

        stack_free(stack);
}

Lambda *get_redex_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Stack *stack = stack_init();

        if (stack == NULL)
                return NULL;

        Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        Lambda *entry = top->term;
                        stack_push(stack, entry);

                        break;

                case LAMBDA_SHORTCUT:
                        break;

                case LAMBDA_VARIABLE:
                        break;
                        
                case LAMBDA_ABSTRACTION:
                        Lambda *body = top->body;
                        stack_push(stack, body);

                        break;

                case LAMBDA_APPLICATION:
                        if (is_redex(top)) {
                                stack_free(stack);
                                return top;
                        }

                        Lambda *right = top->right;
                        Lambda *left = top->left;

                        stack_push(stack, right);
                        stack_push(stack, left);

                        break;

                case LAMBDA_NUMERAL:
                        break;
                }

                top = (Lambda *)stack_pop(stack);
        }

        stack_free(stack);

        return NULL;
}

bool is_redex(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        if (lambda->type != LAMBDA_APPLICATION)
                return false;

        Lambda *left = lambda->left;

        if (left == NULL)
                return false;

        return left->type == LAMBDA_ABSTRACTION;
}