#include <stdio.h>

#include "alpha_rename.h"
#include "stack.h"
#include "printing.h"
#include "variable_capture.h"

struct BindsParam {
        Lambda *lambda;
        Stack *inner_binds;
        struct Variable old_bind;
};

static Stack *get_inner_binds(Lambda *lambda);

static void get_binds_recursive(struct BindsParam param);
static void get_binds_variable(struct BindsParam param);
static void get_binds_abstraction(struct BindsParam param);
static void get_binds_application(struct BindsParam param);

struct RenameParam {
        Lambda *lambda;
        struct Variable old_bind;
        struct Variable new_bind;
};

static void rename_recursive(struct RenameParam param);

static void rename_variable(struct RenameParam param);
static void rename_abstraction(struct RenameParam param);
static void rename_application(struct RenameParam param);

void alpha_rename(Lambda *capture, Lambda *application)
{
        if (capture == NULL || application == NULL)
                return;

        Stack *right_fv = get_free_variables(application->application.right);
        Stack *inner_binds = get_inner_binds(capture);

        if (right_fv == NULL || inner_binds == NULL) {
                stack_free(right_fv);
                stack_free(inner_binds);
        }

        struct Variable old_variable = capture->abstraction.binding;

        struct Variable new_variable = {
                .letter = old_variable.letter,
                .subscript = -1
        };

        for (int i = -1; i < 1000000; i++) {
                new_variable.subscript = i;

                if (!stack_search(right_fv, new_variable)
                 && !stack_search(inner_binds, new_variable))
                        break;
        }

        if (new_variable.subscript == 1000000)
                return;

        struct RenameParam param = {
                .lambda = capture->abstraction.body,
                .old_bind = old_variable,
                .new_bind = new_variable
        };

        rename_recursive(param);

        capture->abstraction.binding = new_variable;

        stack_free(right_fv);
        stack_free(inner_binds);
}

void rename_recursive(struct RenameParam param)
{
        Lambda *lambda = param.lambda;

        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_BIND:
                break;
        
        case LAMBDA_SHORTCUT:
                break;
        
        case LAMBDA_VARIABLE:
                rename_variable(param);
                break;

        case LAMBDA_ABSTRACTION:
                rename_abstraction(param);
                break;

        case LAMBDA_APPLICATION:
                rename_application(param);
                break;
        }
}

Stack *get_inner_binds(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        if (lambda->type != LAMBDA_ABSTRACTION)
                return NULL;

        Stack *inner_binds = stack_init();

        if (inner_binds == NULL)
                return NULL;

        struct BindsParam param = {
                .lambda = lambda->abstraction.body,
                .inner_binds = inner_binds,
                .old_bind = lambda->abstraction.binding
        };

        get_binds_recursive(param);

        return inner_binds;
}

void rename_variable(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable old_bind = param.old_bind;
        struct Variable new_bind = param.new_bind;

        if (variable_compare(lambda->variable, old_bind))
                lambda->variable = new_bind;
}

void rename_abstraction(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable old_bind = param.old_bind;
        struct Variable new_bind = param.new_bind;

        if (!variable_compare(lambda->abstraction.binding, old_bind)) {
                struct RenameParam param = {
                        .lambda = lambda->abstraction.body,
                        .old_bind = old_bind,
                        .new_bind = new_bind
                };

                rename_recursive(param);
        }
}

void rename_application(struct RenameParam param)
{
        Lambda *lambda = param.lambda;
        struct Variable old_bind = param.old_bind;
        struct Variable new_bind = param.new_bind;

        struct RenameParam left_param = {
                .lambda = lambda->application.left,
                .old_bind = old_bind,
                .new_bind = new_bind
        };

        struct RenameParam right_param = {
                .lambda = lambda->application.right,
                .old_bind = old_bind,
                .new_bind = new_bind
        };

        rename_recursive(left_param);
        rename_recursive(right_param);

}

void get_binds_recursive(struct BindsParam param)
{
        Lambda *lambda = param.lambda;

        switch (lambda->type) {
        case LAMBDA_SHORTCUT:
                break;

        case LAMBDA_BIND:
                break;

        case LAMBDA_VARIABLE:
                get_binds_variable(param);
                break;

        case LAMBDA_ABSTRACTION:
                get_binds_abstraction(param);
                break;

        case LAMBDA_APPLICATION:
                get_binds_application(param);
                break;
        }
}

void get_binds_variable(struct BindsParam param)
{
        Lambda *lambda = param.lambda;
        Stack *inner_binds = param.inner_binds;
        struct Variable old_bind = param.old_bind;

        struct Variable variable = lambda->variable;

        if (!variable_compare(old_bind, variable)
         && !stack_search(inner_binds, variable)) {
                stack_push(inner_binds, variable);
        }
}

void get_binds_abstraction(struct BindsParam param)
{
        Lambda *lambda = param.lambda;
        Stack *inner_binds = param.inner_binds;
        struct Variable old_bind = param.old_bind;

        struct Variable variable = lambda->abstraction.binding;

        if (!variable_compare(old_bind, variable)
         && !stack_search(inner_binds, variable)) {
                stack_push(inner_binds, variable);
        }
}

void get_binds_application(struct BindsParam param)
{
        Lambda *lambda = param.lambda;
        Stack *inner_binds = param.inner_binds;
        struct Variable old_bind = param.old_bind;
        
        struct BindsParam left_param = {
                .lambda = lambda->application.left,
                .inner_binds = inner_binds,
                .old_bind = old_bind
        };

        struct BindsParam right_param = {
                .lambda = lambda->application.right,
                .inner_binds = inner_binds,
                .old_bind = old_bind
        };

        get_binds_recursive(left_param);
        get_binds_recursive(right_param);
}