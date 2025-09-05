#include <stdio.h>
#include <string.h>

#include "alpha_equivalence.h"
#include "duplicate.h"
#include "stack.h"

struct RenameParam {
        Lambda *lambda;
        Stack *free_variables;
        Stack *bindings;
};

static void get_free_variables(struct RenameParam param);

static void get_fv_bind(struct RenameParam param);
static void get_fv_variable(struct RenameParam param);
static void get_fv_abstraction(struct RenameParam param);
static void get_fv_application(struct RenameParam param);

static bool capture_check(struct RenameParam param);
static bool capture_check_abstraction(struct RenameParam param);
static bool capture_check_application(struct RenameParam param);

static void rename_abstraction(struct RenameParam param);

static void get_inner_variables(Lambda *lambda, char *old_binding, Stack *inner_variables);
static void rename_bound_variable(Lambda *lambda, char *old_binding, char *new_binding);

bool variable_capture_check(Lambda *lambda)
{
        if (lambda == NULL)
                return false;

        if (lambda->type != LAMBDA_APPLICATION)
                return false;

        Lambda *left = lambda->application.left;
        Lambda *right = lambda->application.right;

        if (left->type != LAMBDA_ABSTRACTION)
                return false;

        Stack *free_variables = stack_init(32);
        Stack *bindings = stack_init(32);

        if (free_variables == NULL || bindings == NULL) {
                stack_free(bindings);
                stack_free(free_variables);
                return false;
        }

        struct RenameParam get_fv_param = {
                .lambda = right,
                .free_variables = free_variables,
                .bindings = bindings
        };

        get_free_variables(get_fv_param);

        if (stack_peek(free_variables) == NULL) {
                stack_free(bindings);
                stack_free(free_variables);
                return false;
        }

        struct RenameParam rename_param = {
                .lambda = left->abstraction.body,
                .free_variables = free_variables,
                .bindings = bindings
        };

        bool capture = capture_check(rename_param);

        stack_free(bindings);
        stack_free(free_variables);

        return capture;
}

bool capture_check(struct RenameParam param)
{
        Lambda *lambda = param.lambda;

        if (lambda == NULL)
                return false;

        switch (lambda->type) {
        case LAMBDA_BIND:
                return false;

        case LAMBDA_VARIABLE:
                return false;

        case LAMBDA_ABSTRACTION:
                return capture_check_abstraction(param);

        case LAMBDA_APPLICATION:
                return capture_check_application(param);
        }

        return false;
}

bool capture_check_abstraction(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bindings = param.bindings;

        char *binding = lambda->abstraction.binding;
        Lambda *body = lambda->abstraction.body;

        char *free_variable = stack_search(free_variables, binding);

        bool capture = free_variable != NULL;

        if (capture) {
                rename_abstraction(param);
                binding = lambda->abstraction.binding;
        }

        stack_push(bindings, binding);

        struct RenameParam body_param = {
                .lambda = lambda->abstraction.body,
                .free_variables = free_variables,
                .bindings = bindings
        };

        bool body_capture = capture_check(body_param);

        stack_pop(bindings);

        return capture || body_capture;
}

bool capture_check_application(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *bindings = param.bindings;

        struct RenameParam left_param = {
                .lambda = lambda->application.left,
                .free_variables = free_variables,
                .bindings = bindings
        };

        bool left_capture = capture_check(left_param);

        struct RenameParam right_param = {
                .lambda = lambda->application.right,
                .free_variables = free_variables,
                .bindings = bindings
        };

        bool right_capture = capture_check(right_param);

        return left_capture || right_capture;
}

void rename_abstraction(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        Stack *free_variables = param.free_variables;
        Stack *outer_bindings = param.bindings;

        Stack *inner_variables = stack_init(32);

        if (inner_variables == NULL)
                return;

        char *old_binding = lambda->abstraction.binding;
        Lambda *body = lambda->abstraction.body;

        get_inner_variables(body, old_binding, inner_variables);

        unsigned i = 0;
        char new_binding[32];
        bool condition;

        do {
                sprintf(new_binding, "x%u", i);
                i++;

                condition = stack_search(free_variables, new_binding)
                         || stack_search(outer_bindings, new_binding)
                         || stack_search(inner_variables, new_binding);
        } while (condition);

        stack_free(inner_variables);

        rename_bound_variable(body, old_binding, new_binding);

        free(old_binding);
        lambda->abstraction.binding = my_strdup(new_binding);
}

void rename_bound_variable(Lambda *lambda, char *old_binding, char *new_binding)
{
        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_VARIABLE:
                char *variable = lambda->variable;

                if (strcmp(old_binding, variable) == 0) {
                        free(variable);
                        lambda->variable = my_strdup(new_binding);
                }

                break;

        case LAMBDA_ABSTRACTION:
                char *binding = lambda->abstraction.binding;
                Lambda *body = lambda->abstraction.body;

                if (strcmp(old_binding, binding) != 0)
                        rename_bound_variable(body, old_binding, new_binding);

                break;

        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                rename_bound_variable(left, old_binding, new_binding);
                rename_bound_variable(right, old_binding, new_binding);
                
                break;
        }
}

void get_inner_variables(Lambda *lambda, char *old_binding, Stack *inner_variables)
{
        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_VARIABLE:
                char *variable = lambda->variable;

                if (strcmp(variable, old_binding) == 0)
                        break;

                if (!stack_search(inner_variables, variable))
                        stack_push(inner_variables, variable);
                
                break;

        case LAMBDA_ABSTRACTION:
                char *binding = lambda->abstraction.binding;
                Lambda *body = lambda->abstraction.body;

                if (!stack_search(inner_variables, binding))
                       stack_push(inner_variables, binding);

                get_inner_variables(body, old_binding, inner_variables);

                break;

        case LAMBDA_APPLICATION:
                Lambda *left = lambda->application.left;
                Lambda *right = lambda->application.right;

                get_inner_variables(left, old_binding, inner_variables);
                get_inner_variables(right, old_binding, inner_variables);

                break;
        }
}

void get_free_variables(struct RenameParam param)
{
        if (param.lambda == NULL)
                return;

        switch (param.lambda->type) {
        case LAMBDA_BIND:
                get_fv_bind(param);
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
        }
}

void get_fv_bind(struct RenameParam param)
{
        struct RenameParam term_param = {
                .lambda = param.lambda->bind.term,
                .free_variables = param.free_variables,
                .bindings = param.bindings
        };

        get_free_variables(term_param);
}

void get_fv_variable(struct RenameParam param)
{
        char *variable = param.lambda->variable;

        char *binding = stack_search(param.bindings, variable);
        char *free_variable = stack_search(param.free_variables, variable);

        if (binding == NULL && free_variable == NULL)
                stack_push(param.free_variables, variable);
}

void get_fv_abstraction(struct RenameParam param)
{
        char *binding = param.lambda->abstraction.binding;

        stack_push(param.bindings, binding);

        struct RenameParam body_param = {
                .lambda = param.lambda->abstraction.body,
                .free_variables = param.free_variables,
                .bindings = param.bindings
        };

        get_free_variables(body_param);

        stack_pop(param.bindings);
}

void get_fv_application(struct RenameParam param)
{
        Lambda *left = param.lambda->application.left;
        Lambda *right = param.lambda->application.right;

        struct RenameParam left_param = {
                .lambda = left,
                .free_variables = param.free_variables,
                .bindings = param.bindings
        };

        get_free_variables(left_param);

        struct RenameParam right_param = {
                .lambda = right,
                .free_variables = param.free_variables,
                .bindings = param.bindings
        };

        get_free_variables(right_param);
}