#pragma once

#include "variable.h"

typedef enum LambdaExprType {
        LAMBDA_ENTRY,
        LAMBDA_SHORTCUT,
        LAMBDA_VARIABLE,
        LAMBDA_ABSTRACTION,
        LAMBDA_APPLICATION,
        LAMBDA_NUMERAL
} LambdaExprType;

typedef struct Lambda Lambda;

struct Lambda {
        LambdaExprType type;

        union {
                char *entry;
                char *shortcut;
                struct Variable variable;
                Lambda *left;
                int numeral;
        };

        union {
                Lambda *term;
                Lambda *body;
                Lambda *right;
        };
};

Lambda *lambda_parse(const char *str);
void lambda_free(Lambda *lambda);