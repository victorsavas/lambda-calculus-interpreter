#pragma once

#include "variable.h"

typedef enum LambdaExprType {
        LAMBDA_BIND,
        LAMBDA_SHORTCUT,
        LAMBDA_VARIABLE,
        LAMBDA_ABSTRACTION,
        LAMBDA_APPLICATION
} LambdaExprType;

typedef struct Lambda Lambda;

struct Lambda {
        LambdaExprType type;

        union {
                struct {
                        char *shortcut;
                        Lambda *term;
                } bind;

                char *shortcut;

                struct Variable variable;

                struct {
                        struct Variable binding;
                        Lambda *body;
                } abstraction;

                struct {
                        Lambda *left;
                        Lambda *right;
                } application;
                
        };
};

Lambda *lambda_parse(const char *str);
void lambda_free(Lambda *lambda);