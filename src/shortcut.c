#include <stdlib.h>

#include "duplicate.h"
#include "shortcut.h"

struct ReplaceParam {
        Lambda *lambda;
        HashTable *table;
};

static bool replace_recursive(struct ReplaceParam param);

static bool replace_bind(struct ReplaceParam param);
static bool replace_shortcut(struct ReplaceParam param);
static bool replace_variable(struct ReplaceParam param);
static bool replace_abstraction(struct ReplaceParam param);
static bool replace_application(struct ReplaceParam param);

bool replace_shortcuts(Lambda *lambda, HashTable *table)
{
        if (table == NULL || lambda == NULL)
                return false;

        struct ReplaceParam param = {
                .lambda = lambda,
                .table = table,
        };

        bool success = replace_recursive(param);

        return success;
}

bool replace_recursive(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;

        if (lambda == NULL || table == NULL)
                return false;
        
        switch (lambda->type) {
        case LAMBDA_VARIABLE:
                return true;

        case LAMBDA_BIND:
                return replace_bind(param);

        case LAMBDA_SHORTCUT:
                return replace_shortcut(param);
        
        case LAMBDA_ABSTRACTION:
                return replace_abstraction(param);

        case LAMBDA_APPLICATION:
                return replace_application(param);
        }

        return false;
}

bool replace_bind(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;

        struct ReplaceParam bind_param = {
                .lambda = lambda->bind.term,
                .table = table
        };

        return replace_recursive(bind_param);
}

bool replace_shortcut(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;

        const char *shortcut = lambda->shortcut;

        Lambda *binding = hashtable_search(table, shortcut);

        if (binding == NULL)
                return false;
                
        Lambda *duplicate = lambda_duplicate(binding->bind.term);

        if (duplicate == NULL)
                return false;

        free(shortcut);

        *lambda = *duplicate;

        free(duplicate);

        return true;
}

bool replace_abstraction(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;

        struct ReplaceParam abstraction_param = {
                .lambda = lambda->abstraction.body,
                .table = table
        };

        return replace_recursive(abstraction_param);
}

bool replace_application(struct ReplaceParam param)
{
        Lambda *lambda = param.lambda;
        HashTable *table = param.table;

        Lambda *left = lambda->application.left;
        Lambda *right = lambda->application.right;

        struct ReplaceParam left_param = {
                .lambda = left,
                .table = table
        };

        bool left_code = replace_recursive(left_param);

        if (!left_code)
                return false;

        struct ReplaceParam right_param = {
                .lambda = right,
                .table = table
        };

        bool right_code = replace_recursive(right_param);

        return right_code;
}