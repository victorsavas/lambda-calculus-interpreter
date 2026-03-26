#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alpha_rename.h"
#include "duplicate.h"
#include "printing.h"
#include "reduction.h"
#include "variable.h"
#include "variable_capture.h"

struct ReductionParam {
        Lambda *lambda;
        struct Variable binding;
        Lambda *right;
};

static Lambda *get_call_by_name(Lambda *lambda);
static Lambda *get_call_by_value(Lambda *lambda);

static void beta_reduction(Lambda *application);
static void beta_reduction_recursive(struct ReductionParam param);

static void reduce_variable(struct ReductionParam param);
static void reduce_abstraction(struct ReductionParam param);
static void reduce_application(struct ReductionParam param);

bool lambda_is_normal(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        Lambda *application = get_call_by_name(lambda);

        return application == NULL;
}

Lambda *lambda_reduce(Lambda *lambda, Mode mode, unsigned iterations)
{
        if (!(mode & MODE_REDUCE))
                return lambda;

        if (lambda == NULL)
                return NULL;
        
        for (unsigned i = 0; i < iterations; i++) {
                Lambda *application;

                if (mode & MODE_CALL_BY_VALUE)
                        application = get_call_by_value(lambda);
                else
                        application = get_call_by_name(lambda);

                // Normal form reached
                if (application == NULL)
                        break;

                if (mode & MODE_VERBOSE) {
                        printf("%6u: ", i);
                        lambda_print(lambda);
                        printf(" -> [");
                        lambda_print(application);
                        printf("]\n");
                }

                // Tests for variable capture
                Lambda *capture = variable_capture(application);

                if (capture == NULL) {
                        beta_reduction(application);
                }
                else
                        alpha_rename(capture, application);
        }

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

Lambda *get_call_by_name(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        switch (lambda->type) {
        case LAMBDA_BIND:
                return get_call_by_name(lambda->bind.term);
        
        case LAMBDA_SHORTCUT:
        case LAMBDA_VARIABLE:
                return NULL;

        case LAMBDA_ABSTRACTION:
                return get_call_by_name(lambda->abstraction.body);

        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                if (left == NULL || right == NULL)
                        return NULL;

                if (left->type == LAMBDA_ABSTRACTION)
                        return lambda;

                Lambda *left_reduction = get_call_by_name(left);

                if (left_reduction != NULL)
                        return left_reduction;

                Lambda *right_reduction = get_call_by_name(right);

                return right_reduction;
        }

        return NULL;
}

Lambda *get_call_by_value(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        switch (lambda->type) {
        case LAMBDA_BIND:
                return get_call_by_value(lambda->bind.term);

        case LAMBDA_SHORTCUT:
        case LAMBDA_VARIABLE:
                return NULL;

        case LAMBDA_ABSTRACTION:
                return get_call_by_value(lambda->abstraction.body);
                
        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                if (left == NULL || right == NULL)
                        return NULL;

                Lambda *right_reduction = get_call_by_value(right);

                if (right_reduction != NULL)
                        return right_reduction;

                if (left->type == LAMBDA_ABSTRACTION)
                        return lambda;

                Lambda *left_reduction = get_call_by_value(left);

                return left_reduction;
        }

        return NULL;
}