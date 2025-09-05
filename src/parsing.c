#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parsing.h"

typedef enum TokenType {
        TOKEN_END,
        TOKEN_VARIABLE,
        TOKEN_EQUALS,
        TOKEN_LEFT_PARENTHESIS,
        TOKEN_RIGHT_PARENTHESIS,
        TOKEN_LAMBDA,
        TOKEN_DOT,
        TOKEN_INVALID
} TokenType;

struct Token {
        TokenType type;
        const char *start;
        size_t size;
};

struct ParseParam {
        const char **str;
        struct Token *token;
        bool bind;
        bool parenthesis;
};

static void get_token(struct Token *token, const char **c);
static char *duplicate_token_string(struct Token *token);

static Lambda *parse_expression(struct ParseParam param);

static Lambda *parse_application(struct ParseParam param);
static Lambda *parse_abstraction(struct ParseParam param);

static Lambda *parse_variable(struct Token *token);
static Lambda *parse_binding(Lambda *left, struct ParseParam param);
static Lambda *parse_right_parenthesis(Lambda *lambda, bool parenthesis);
static Lambda *parse_end(Lambda *lambda, bool parenthesis);

Lambda *lambda_parse(const char *str)
{
        if (str == NULL)
                return NULL;

        struct Token token;

        struct ParseParam param = {
                .str = &str,
                .token = &token,
                .bind = true,
                .parenthesis = false,
        };

        Lambda *lambda = parse_expression(param);

        if (token.type != TOKEN_END) {
                lambda_free(lambda);
                return NULL;
        }

        return lambda;
}

void lambda_free(Lambda *lambda)
{
        if (lambda == NULL)
                return;

        switch (lambda->type) {
        case LAMBDA_BIND:
                free(lambda->bind.name);
                lambda_free(lambda->bind.term);
                break;

        case LAMBDA_VARIABLE:
                free(lambda->variable);
                break;

        case LAMBDA_ABSTRACTION:
                free(lambda->abstraction.binding);
                lambda_free(lambda->abstraction.body);
                break;

        case LAMBDA_APPLICATION:
                lambda_free(lambda->application.left);
                lambda_free(lambda->application.right);
                break;
        }

        free(lambda);
}

Lambda *parse_expression(struct ParseParam param)
{
        get_token(param.token, param.str);

        switch (param.token->type) {
        case TOKEN_VARIABLE:
        case TOKEN_LEFT_PARENTHESIS:
                return parse_application(param);

        case TOKEN_LAMBDA:
                return parse_abstraction(param);

        case TOKEN_RIGHT_PARENTHESIS:
        case TOKEN_END:
                printf("Syntax error. Expression expected.\n");
                return NULL;

        case TOKEN_DOT:
        case TOKEN_EQUALS:
                printf("Syntax error. Invalid operator.\n");
                return NULL;

        case TOKEN_INVALID:
                printf("Syntax error. Invalid token.\n");
                return NULL;
        }

        return NULL;
}

Lambda *parse_application(struct ParseParam param)
{
        Lambda *left = NULL;
        Lambda *right;

        while (true) {
                switch (param.token->type) {
                case TOKEN_END:
                        return parse_end(left, param.parenthesis);

                case TOKEN_VARIABLE:
                        right = parse_variable(param.token);
                        break;
        
                case TOKEN_EQUALS:
                        return parse_binding(left, param);

                case TOKEN_LEFT_PARENTHESIS:
                        struct ParseParam right_param = {
                                .str = param.str,
                                .token = param.token,
                                .bind = false,
                                .parenthesis = true
                        };

                        right = parse_expression(right_param);
                        break;
                
                case TOKEN_RIGHT_PARENTHESIS:
                        return parse_right_parenthesis(left, param.parenthesis);

                case TOKEN_LAMBDA:
                case TOKEN_DOT:
                        printf("Syntax error. Invalid operator.\n");
                        lambda_free(left);
                        return NULL;

                case TOKEN_INVALID:
                        printf("Syntax error. Invalid character.\n");
                        lambda_free(left);
                        return NULL;
                }

                if (right == NULL) {
                        lambda_free(left);
                        return NULL;
                }

                if (left == NULL) {
                        left = right;

                        get_token(param.token, param.str);
                        continue;
                }

                Lambda *lambda = malloc(sizeof(*lambda));

                if (lambda == NULL) {
                        lambda_free(left);
                        lambda_free(right);
                        return NULL;
                }

                lambda->type = LAMBDA_APPLICATION;
                lambda->application.left = left;
                lambda->application.right = right;

                left = lambda;

                param.bind = false;
                get_token(param.token, param.str);
        }
}

Lambda *parse_abstraction(struct ParseParam param)
{
        get_token(param.token, param.str);

        if (param.token->type != TOKEN_VARIABLE) {
                printf("Syntax error. Expected variable token.\n");
                return NULL;
        }

        Lambda *lambda = parse_variable(param.token);
        
        if (lambda == NULL)
                return NULL;

        char *variable = lambda->variable;

        lambda->type = LAMBDA_ABSTRACTION;
        lambda->abstraction.binding = variable;

        get_token(param.token, param.str);

        if (param.token->type != TOKEN_DOT) {
                printf("Syntax error. Expected dot operator.\n");

                free(variable);
                free(lambda);
                return NULL;
        }

        param.bind = false;

        Lambda *body = parse_expression(param);

        if (body == NULL) {
                free(variable);
                free(lambda);
                return NULL;
        }

        lambda->abstraction.body = body;

        return lambda;
}

Lambda *parse_variable(struct Token *token)
{
        Lambda *lambda = malloc(sizeof(*lambda));

        if (lambda == NULL)
                return NULL;

        lambda->type = LAMBDA_VARIABLE;
                
        char *variable = duplicate_token_string(token);

        if (variable == NULL) {
                free(lambda);
                return NULL;
        }

        lambda->variable = variable;

        return lambda;
}

Lambda *parse_binding(Lambda *left, struct ParseParam param)
{
        if (left == NULL)
                return NULL;

        if (!param.bind || left->type != LAMBDA_VARIABLE) {
                printf("Syntax error: invalid assignment operator.\n");
                lambda_free(left);
                return NULL;
        }

        param.bind = false;
        param.parenthesis = false;
                        
        Lambda *right = parse_expression(param);

        if (right == NULL) {
                lambda_free(left);
                return NULL;
        }

        char *variable = left->variable;

        left->type = LAMBDA_BIND;
        left->bind.name = variable;
        left->bind.term = right;

        return left;
}

Lambda *parse_end(Lambda *lambda, bool parenthesis)
{
        if (parenthesis) {
                printf("Syntax error. Missing right parenthesis.\n");
                lambda_free(lambda);
                return NULL;
        }

        return lambda;
}

Lambda *parse_right_parenthesis(Lambda *lambda, bool parenthesis)
{
        if (!parenthesis) {
                printf("Syntax error: unexpected parenthesis.\n");
                lambda_free(lambda);
                return NULL;
        }

        return lambda;
}

void get_token(struct Token *token, const char **c)
{
        if (token == NULL || c == NULL)
                return;

        *token = (struct Token){
                .type = TOKEN_END,
                .start = NULL,
                .size = 0
        };

        while (isspace(**c))
                (*c)++;

        if (**c == '\0')
                return;

        switch (**c) {
        case '(':
                token->type = TOKEN_LEFT_PARENTHESIS;
                break;

        case ')':
                token->type = TOKEN_RIGHT_PARENTHESIS;
                break;

        case '\\':
                token->type = TOKEN_LAMBDA;
                break;

        case '.':
                token->type = TOKEN_DOT;
                break;

        case '=':
                token->type = TOKEN_EQUALS;
                break;
        }

        if (token->type != TOKEN_END) {
                token->start = *c;
                token->size = 1;

                (*c)++;

                return;
        }

        token->type = TOKEN_VARIABLE;
        token->start = *c;

        while (isalnum(**c) || **c == '_')
                (*c)++;

        token->size = *c - token->start;

        if (token->size == 0)
                token->type = TOKEN_INVALID;
}

char *duplicate_token_string(struct Token *token)
{
        if (token == NULL)
                return NULL;

        char *duplicate = malloc(token->size + 1);

        if (duplicate == NULL)
                return NULL;

        strncpy(duplicate, token->start, token->size);
        duplicate[token->size] = '\0';

        return duplicate;
}