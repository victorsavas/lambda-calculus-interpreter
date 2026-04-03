#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alpha_rename.h"
#include "ansi_escapes.h"
#include "duplicate.h"
#include "printing.h"
#include "reduction.h"
#include "variable.h"
#include "variable_capture.h"

#define LONG_CYCLE 10000

struct ReductionParam {
        Lambda *lambda;
        struct Variable binding;
        Lambda *right;
};

static Lambda *get_leftmost(Lambda *lambda);
static Lambda *get_rightmost(Lambda *lambda);

static void beta_reduction(Lambda *application);
static void beta_reduction_recursive(struct ReductionParam param);

static void reduce_variable(struct ReductionParam param);
static void reduce_abstraction(struct ReductionParam param);
static void reduce_application(struct ReductionParam param);

bool lambda_is_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        Lambda *application = get_leftmost(lambda);

        return application == NULL;
}

Lambda *lambda_reduce(Lambda *lambda, Mode mode, unsigned iterations)
{
        if (lambda == NULL)
                return NULL;
        
        bool normal_form = false;

        if (!(mode & (MODE_REDUCE | MODE_VERBOSE))) {
                lambda_print(lambda, NULL);
                
                normal_form = lambda_is_normal(lambda);

                if (normal_form)
                        printf(ANSI_BLUE " (Normal form.)\n" ANSI_RESET);
                else
                        printf("\n");

                return lambda;
        }

        unsigned i;
        
        for (i = 0; i < iterations; i++) {
                Lambda *application;

                if (mode & MODE_RIGHTMOST)
                        application = get_rightmost(lambda);
                else
                        application = get_leftmost(lambda);

                if (application == NULL) {
                        normal_form = true;
                        break;
                }

                if (mode & MODE_VERBOSE) {
                        printf(ANSI_GREY "%-5u " ANSI_RESET, i + 1);
                        lambda_print(lambda, application);
                        printf("\n");
                } else if ((i + 1) % LONG_CYCLE == 0)
                        printf(".\n");

                // Tests for variable capture
                Lambda *capture = variable_capture(application);

                if (capture == NULL)
                        beta_reduction(application);
                else
                        alpha_rename(capture, application);
        }

        lambda_print(lambda, NULL);

        if (normal_form)
                printf(ANSI_BLUE " (Normal form reached after %d steps.)\n" ANSI_RESET, i);
        else
                printf(ANSI_BLUE " (Normal form not reached.)\n" ANSI_RESET);

        return lambda;
}

void beta_reduction(Lambda *application)
{
        if (application == NULL)
                return;

        if (application->type != LAMBDA_APPLICATION)
                return;

        Lambda *left = application->application.left;
        Lambda *right = application->application.right;

        if (left->type != LAMBDA_ABSTRACTION)
                return;

        struct Variable binding = left->abstraction.binding;
        Lambda *body = left->abstraction.body;

        struct ReductionParam param = {
                .lambda = body,
                .binding = binding,
                .right = right
        };

        beta_reduction_recursive(param);

        lambda_free(right);

        *application = *body;
        free(body);
}

void beta_reduction_recursive(struct ReductionParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable binding = param.binding;
        Lambda *right = param.right;

        if (lambda == NULL || right == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_VARIABLE:
                reduce_variable(param);
                break;

        case LAMBDA_ABSTRACTION:
                reduce_abstraction(param);
                break;

        case LAMBDA_APPLICATION:
                reduce_application(param);
                break;
        }
}

void reduce_variable(struct ReductionParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable binding = param.binding;
        Lambda *right = param.right;

        struct Variable variable = lambda->variable;
                
        if (!variable_compare(variable, binding))
                return;

        Lambda *duplicate = lambda_duplicate(right);

        if (duplicate == NULL)
                return;

        *lambda = *duplicate;

        free(duplicate);
}

void reduce_abstraction(struct ReductionParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable binding = param.binding;
        struct Variable variable = lambda->abstraction.binding;

        if (variable_compare(binding, variable))
                return;

        struct ReductionParam body_param = {
                .lambda = lambda->abstraction.body,
                .binding = binding,
                .right = param.right
        };

        beta_reduction_recursive(body_param);
}

void reduce_application(struct ReductionParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable binding = param.binding;
        Lambda *right = param.right;

        struct ReductionParam left_param = {
                .lambda = lambda->application.left,
                .binding = binding,
                .right = right
        };

        beta_reduction_recursive(left_param);

        struct ReductionParam right_param = {
                .lambda = lambda->application.right,
                .binding = binding,
                .right = right
        };

        beta_reduction_recursive(right_param);
}

Lambda *get_leftmost(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Lambda *leftmost = lambda;

        while (1) {
                switch (leftmost->type) {
                case LAMBDA_BIND:
                        leftmost = leftmost->bind.term;
                        continue;
                
                case LAMBDA_SHORTCUT:
                case LAMBDA_VARIABLE:
                        return NULL;

                case LAMBDA_ABSTRACTION:
                        leftmost = leftmost->abstraction.body;
                        continue;

                case LAMBDA_APPLICATION:
                        Lambda *left = leftmost->application.left;
                        Lambda *right = leftmost->application.right;

                        if (left == NULL || right == NULL)
                                return NULL;

                        if (left->type == LAMBDA_ABSTRACTION)
                                return leftmost;

                        Lambda *left_reduction = get_leftmost(left);

                        if (left_reduction != NULL)
                                return left_reduction;

                        leftmost = right;

                        continue;
                }
        }

        return NULL;
}

Lambda *get_rightmost(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        Lambda *rightmost = lambda;

        while (1) {
                switch (rightmost->type) {
                case LAMBDA_BIND:
                        rightmost = rightmost->bind.term;
                        continue;

                case LAMBDA_SHORTCUT:
                case LAMBDA_VARIABLE:
                        return NULL;

                case LAMBDA_ABSTRACTION:
                        rightmost = rightmost->abstraction.body;
                        continue;

                case LAMBDA_APPLICATION:
                        Lambda *left = rightmost->application.left;
                        Lambda *right = rightmost->application.right;

                        if (left == NULL || right == NULL)
                                return NULL;

                        Lambda *right_reduction = get_rightmost(right);

                        if (right_reduction != NULL)
                                return right_reduction;

                        if (left->type == LAMBDA_ABSTRACTION)
                                return left;
                        
                        rightmost = left;

                        continue;
                }
        }

        return NULL;
}