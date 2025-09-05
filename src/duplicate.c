#include <stdlib.h>
#include <string.h>

#include "duplicate.h"

static Lambda *duplicate_bind(Lambda *lambda);
static Lambda *duplicate_variable(Lambda *lambda);
static Lambda *duplicate_abstraction(Lambda *lambda);
static Lambda *duplicate_application(Lambda *lambda);

Lambda *lambda_duplicate(Lambda *lambda)
{
        if (lambda == NULL)
                return NULL;

        switch (lambda->type) {
        case LAMBDA_BIND:
                return duplicate_bind(lambda);

        case LAMBDA_VARIABLE:
                return duplicate_variable(lambda);

        case LAMBDA_ABSTRACTION:
                return duplicate_abstraction(lambda);

        case LAMBDA_APPLICATION:
                return duplicate_application(lambda);
        }

        return NULL;
}

Lambda *duplicate_bind(Lambda *lambda)
{
        Lambda *duplicate;
        duplicate = malloc(sizeof(*duplicate));
        
        if (duplicate == NULL)
                return NULL;

        char *name = my_strdup(lambda->bind.name);

        if (name == NULL) {
                free(duplicate);
                return NULL;
        }

        Lambda *term = lambda_duplicate(lambda->bind.term);

        if (term == NULL) {
                free(duplicate);
                free(name);
                return NULL;
        }

        duplicate->type = LAMBDA_BIND;
        duplicate->bind.name = name;
        duplicate->bind.term = term;

        return duplicate;
}

Lambda *duplicate_variable(Lambda *lambda)
{
        Lambda *duplicate;
        duplicate = malloc(sizeof(*duplicate));

        if (duplicate == NULL)
                return NULL;

        char *variable = my_strdup(lambda->variable);

        if (variable == NULL) {
                free(duplicate);
                return NULL;
        }

        duplicate->type = LAMBDA_VARIABLE;
        duplicate->variable = variable;

        return duplicate;
}

Lambda *duplicate_abstraction(Lambda *lambda)
{
        Lambda *duplicate;
        duplicate = malloc(sizeof(*duplicate));
        
        if (duplicate == NULL)
                return NULL;

        char *binding = my_strdup(lambda->abstraction.binding);

        if (binding == NULL) {
                free(duplicate);
                return NULL;
        }

        Lambda *body = lambda_duplicate(lambda->abstraction.body);

        if (body == NULL) {
                free(duplicate);
                free(binding);
                return NULL;
        }

        duplicate->type = LAMBDA_ABSTRACTION;
        duplicate->abstraction.binding = binding;
        duplicate->abstraction.body = body;

        return duplicate;
}

Lambda *duplicate_application(Lambda *lambda)
{
        Lambda *duplicate;
        duplicate = malloc(sizeof(*duplicate));
        
        if (duplicate == NULL)
                return NULL;

        Lambda *left = lambda_duplicate(lambda->application.left);

        if (left == NULL) {
                free(duplicate);
                return NULL;
        }

        Lambda *right = lambda_duplicate(lambda->application.right);

        if (right == NULL) {
                lambda_free(left);
                free(duplicate);
                return NULL;
        }

        duplicate->type = LAMBDA_APPLICATION;
        duplicate->application.left = left;
        duplicate->application.right = right;

        return duplicate;
}

char *my_strdup(char *str)
{
        if (str == NULL)
                return NULL;

        size_t length = strlen(str) + 1;
        char *duplicate = malloc(length);

        if (duplicate == NULL)
                return NULL;

        strcpy(duplicate, str);

        return duplicate;
}