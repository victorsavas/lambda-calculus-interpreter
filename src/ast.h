#pragma once

#include "variable.h"

typedef enum LambdaExprType {
        LAMBDA_VARIABLE,
        LAMBDA_ABSTRACTION,
        LAMBDA_APPLICATION,
        LAMBDA_SHORTCUT,
        LAMBDA_ENTRY,
        LAMBDA_NUMERAL,
        LAMBDA_INDIRECTION
} LambdaExprType;

typedef struct Lambda Lambda;

struct Lambda {
        int instances;

        LambdaExprType type;

        union {
                struct Variable variable;

                struct {
                        struct Variable bind;
                        Lambda *body;
                };      // abstraction

                struct {
                        Lambda *left;
                        Lambda *right;
                };      // application

                char *shortcut;

                struct {
                        char *entry;
                        Lambda *expression;
                };      // entry

                int numeral;

                // struct {
                //         Lambda *indirection;
                // };      // indirection
        };
};

void lambda_free(Lambda *lambda);