#include <stdio.h>

#include "alpha_rename.h"
#include "stack.h"
#include "printing.h"
#include "variable_capture.h"

#define SUBSCRIPT_LIMIT 1000

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

void alpha_rename(Lambda *capture, Lambda *redex)
{
        if (capture == NULL || redex == NULL)
                return;

        Stack *right_fv = get_free_variables(redex->right);
        Stack *inner_binds = get_inner_binds(capture);

        if (right_fv == NULL || inner_binds == NULL) {
                stack_free(right_fv);
                stack_free(inner_binds);
        }

        struct Variable old_variable = capture->variable;

        struct Variable new_variable = {
                .letter = old_variable.letter,
                .subscript = -1
        };

        for (int i = -1; i < SUBSCRIPT_LIMIT; i++) {
                new_variable.subscript = i;

                if (!stack_search(right_fv, &new_variable, variable_search)
                 && !stack_search(inner_binds, &new_variable, variable_search))
                        break;
        }

        if (new_variable.subscript == SUBSCRIPT_LIMIT)
                return;

        struct RenameParam param = {
                .lambda = capture->body,
                .old_bind = old_variable,
                .new_bind = new_variable
        };

        rename_recursive(param);

        capture->variable = new_variable;

        stack_free(right_fv);
        stack_free(inner_binds);
}

void rename_recursive(struct RenameParam param)
{
        Lambda *lambda = param.lambda;

        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_ENTRY:
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
                .lambda = lambda->body,
                .inner_binds = inner_binds,
                .old_bind = lambda->variable
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

        if (!variable_compare(lambda->variable, old_bind)) {
                struct RenameParam param = {
                        .lambda = lambda->body,
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
                .lambda = lambda->left,
                .old_bind = old_bind,
                .new_bind = new_bind
        };

        struct RenameParam right_param = {
                .lambda = lambda->right,
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

        case LAMBDA_ENTRY:
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

        struct Variable *variable = &lambda->variable;

        if (!variable_compare(old_bind, *variable)
         && !stack_search(inner_binds, variable, variable_search)) {
                stack_push(inner_binds, variable);
        }
}

void get_binds_abstraction(struct BindsParam param)
{
        Lambda *lambda = param.lambda;
        Stack *inner_binds = param.inner_binds;
        struct Variable old_bind = param.old_bind;

        struct Variable *variable = &lambda->variable;

        if (!variable_compare(old_bind, *variable)
         && !stack_search(inner_binds, variable, variable_search)) {
                stack_push(inner_binds, variable);
        }
}

void get_binds_application(struct BindsParam param)
{
        Lambda *lambda = param.lambda;
        Stack *inner_binds = param.inner_binds;
        struct Variable old_bind = param.old_bind;
        
        struct BindsParam left_param = {
                .lambda = lambda->left,
                .inner_binds = inner_binds,
                .old_bind = old_bind
        };

        struct BindsParam right_param = {
                .lambda = lambda->right,
                .inner_binds = inner_binds,
                .old_bind = old_bind
        };

        get_binds_recursive(left_param);
        get_binds_recursive(right_param);
}