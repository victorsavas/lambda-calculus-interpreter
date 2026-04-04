#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "variable_capture.h"
#include "duplicate.h"
#include "stack.h"

struct CaptureParam {
        Lambda *lambda;
        Stack *free_variables;
        Stack *bound_variables;
};

static Lambda *capture_check(struct CaptureParam param);

static Lambda *abstraction_check(struct CaptureParam param);
static Lambda *application_check(struct CaptureParam param);

static void get_fv_recursive(struct CaptureParam param);
static void get_fv_variable(struct CaptureParam param);
static void get_fv_abstraction(struct CaptureParam param);
static void get_fv_application(struct CaptureParam param);
static void get_fv_bind(struct CaptureParam param);

Lambda *variable_capture(Lambda *application)
{
        if (application == NULL)
                return NULL;

        if (application->type != LAMBDA_APPLICATION)
                return NULL;

        Lambda *left = application->left;
        Lambda *right = application->right;

        if (left->type != LAMBDA_ABSTRACTION)
                return NULL;

        Stack *right_fv = get_free_variables(right);

        if (right_fv == NULL)
                return NULL;

        if (stack_peek(right_fv) == NULL){
                stack_free(right_fv);
                return NULL;
        }

        Stack *left_binds = stack_init();

        if (left_binds == NULL) {
                stack_free(right_fv);
                return NULL;
        }

        struct CaptureParam param = {
                .lambda = left,
                .free_variables = right_fv,
                .bound_variables = left_binds
        };

        Lambda *capture = capture_check(param);

        stack_free(right_fv);
        stack_free(left_binds);

        return capture;
}

Stack *get_free_variables(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;
        
        Stack *free_variables = stack_init();
        Stack *bound_variables = stack_init();

        if (free_variables == NULL || bound_variables == NULL) {
                stack_free(free_variables);
                stack_free(bound_variables);

                return NULL;
        }

        struct CaptureParam param = {
                .lambda = lambda,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        get_fv_recursive(param);

        stack_free(bound_variables);

        return free_variables;
}

Lambda *capture_check(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;

        if (lambda == NULL)
                return NULL;

        switch (lambda->type) {
        case LAMBDA_ENTRY:
                return NULL;

        case LAMBDA_SHORTCUT:
                return NULL;

        case LAMBDA_VARIABLE:
                return NULL;

        case LAMBDA_ABSTRACTION:
                return abstraction_check(param);

        case LAMBDA_APPLICATION:
                return application_check(param);
        }

        return NULL;
}

Lambda *abstraction_check(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        struct Variable *binding = &lambda->variable;

        bool capture = stack_search(free_variables, binding, variable_search);

        if (capture)
                return lambda;

        stack_push(bound_variables, binding);

        struct CaptureParam body_param = {
                .lambda = lambda->body,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        Lambda *body_capture = capture_check(body_param);

        stack_pop(bound_variables);

        return body_capture;
}

Lambda *application_check(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        Lambda *left = lambda->left;
        Lambda *right = lambda->right;

        struct CaptureParam left_param = {
                .lambda = left,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        Lambda *left_check = capture_check(left_param);

        if (left_check)
                return left_check;

        struct CaptureParam right_param = {
                .lambda = right,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        Lambda *right_check = capture_check(right_param);

        return right_check;
}

void get_fv_recursive(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;

        switch (lambda->type) {
        case LAMBDA_SHORTCUT:
                break;
        
        case LAMBDA_VARIABLE:
                get_fv_variable(param);
                break;

        case LAMBDA_ABSTRACTION:
                get_fv_abstraction(param);
                break;
        
        case LAMBDA_APPLICATION:
                get_fv_application(param);
                break;
                
        case LAMBDA_ENTRY:
                get_fv_bind(param);
                break;
        }
}

void get_fv_variable(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        struct Variable *variable = &lambda->variable;

        if (!stack_search(bound_variables, variable, variable_search))
                if (!stack_search(free_variables, variable, variable_search))
                        stack_push(free_variables, variable);
}

void get_fv_abstraction(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        struct Variable *variable = &lambda->variable;

        stack_push(bound_variables, variable);

        struct CaptureParam abstraction_param = {
                .lambda = lambda->body,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        get_fv_recursive(abstraction_param);

        stack_pop(bound_variables);
}

void get_fv_application(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        struct CaptureParam left_param = {
                .lambda = lambda->left,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        get_fv_recursive(left_param);

        struct CaptureParam right_param = {
                .lambda = lambda->right,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        get_fv_recursive(right_param);
}

void get_fv_bind(struct CaptureParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bound_variables = param.bound_variables;

        struct CaptureParam bind_param = {
                .lambda = lambda->term,
                .free_variables = free_variables,
                .bound_variables = bound_variables
        };

        get_fv_recursive(bind_param);
}