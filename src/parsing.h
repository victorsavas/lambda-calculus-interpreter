#pragma once

typedef enum LambdaExprType {
        LAMBDA_BIND,
        LAMBDA_VARIABLE,
        LAMBDA_ABSTRACTION,
        LAMBDA_APPLICATION
} LambdaExprType;

typedef struct Lambda Lambda;

struct Lambda {
        LambdaExprType type;

        union {
                struct {
                        char *name;
                        Lambda *term;
                } bind;

                char *variable;

                struct {
                        char *binding;
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