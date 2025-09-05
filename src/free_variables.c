#include "duplicate.h"
#include "free_variables.h"
#include "stack.h"

struct ReplaceParam {
        Lambda *lambda;
        HashTable *table;
        Stack *binds;
};

static bool replace_recursive(struct ReplaceParam param);

static bool replace_variable(struct ReplaceParam param);
static bool replace_abstraction(struct ReplaceParam param);
static bool replace_application(struct ReplaceParam param);

bool lambda_replace_free_variables(Lambda *lambda, HashTable *table)
{
        if (table == NULL || lambda == NULL)
                return false;

        Stack *binds = stack_init(32);

        if (binds == NULL)
                return false;

        struct ReplaceParam param = {
                .lambda = lambda,
                .table = table,
                .binds = binds
        };

        bool success = replace_recursive(param);

        stack_free(binds);

        return success;
}

bool replace_recursive(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;
        Stack *binds = param.binds;

        if (lambda == NULL || table == NULL || binds == NULL)
                return false;
        
        switch (lambda->type) {
        case LAMBDA_BIND:
                param.lambda = lambda->bind.term;
                return replace_recursive(param);

        case LAMBDA_VARIABLE:
                return replace_variable(param);
        
        case LAMBDA_ABSTRACTION:
                return replace_abstraction(param);

        case LAMBDA_APPLICATION:
                return replace_application(param);
        }

        return false;
}

bool replace_variable(struct ReplaceParam param)
{
        char *variable = param.lambda->variable;
        char *binding = stack_search(param.binds, variable);

        if (binding != NULL)
                return true;

        Lambda *free_variable = hashtable_search(param.table, variable);

        if (free_variable == NULL)
                return false;
                
        Lambda *duplicate = lambda_duplicate(free_variable->bind.term);

        if (duplicate == NULL)
                return false;

        free(variable);

        *param.lambda = *duplicate;

        free(duplicate);

        return true;
}

bool replace_abstraction(struct ReplaceParam param)
{
        size_t index = stack_push(param.binds, param.lambda->abstraction.binding);

        if (index == 0)
                return false;

        param.lambda = param.lambda->abstraction.body;

        bool code = replace_recursive(param);

        stack_pop(param.binds);

        return code;
}

bool replace_application(struct ReplaceParam param)
{
        Lambda *left = param.lambda->application.left;
        Lambda *right = param.lambda->application.right;

        param.lambda = left;

        bool left_code = replace_recursive(param);

        if (!left_code)
                return false;

        param.lambda = right;

        bool right_code = replace_recursive(param);

        return right_code;
}