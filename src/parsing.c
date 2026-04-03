#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escape.h"
#include "parsing.h"

typedef enum TokenType {
        TOKEN_END,
        TOKEN_SHORTCUT,
        TOKEN_VARIABLE,
        TOKEN_NUMERAL,
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

static Lambda *parse_shortcut(struct Token *token);
static struct Variable parse_variable(struct ParseParam param);
static Lambda *parse_binding(Lambda *left, struct ParseParam param);
static Lambda *parse_right_parenthesis(Lambda *lambda, bool parenthesis);
static Lambda *parse_end(Lambda *lambda, bool parenthesis);

static Lambda *generate_church_numeral(int integer);

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
                free(lambda->bind.shortcut);
                lambda_free(lambda->bind.term);
                break;

        case LAMBDA_SHORTCUT:
                free(lambda->shortcut);
                break;

        case LAMBDA_ABSTRACTION:
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
        case TOKEN_NUMERAL:
        case TOKEN_VARIABLE:
        case TOKEN_SHORTCUT:
        case TOKEN_LEFT_PARENTHESIS:
                return parse_application(param);

        case TOKEN_LAMBDA:
                return parse_abstraction(param);

        case TOKEN_RIGHT_PARENTHESIS:
        case TOKEN_END:
                printf(ANSI_RED "Syntax error. Expression expected.\n" ANSI_RESET);
                return NULL;

        case TOKEN_DOT:
        case TOKEN_EQUALS:
                printf(ANSI_RED "Syntax error. Invalid operator.\n" ANSI_RESET);
                return NULL;

        case TOKEN_INVALID:
                printf(ANSI_RED "Syntax error. Invalid token.\n" ANSI_RESET);
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

                case TOKEN_NUMERAL:
                        int integer = atoi(param.token->start);
                        right = generate_church_numeral(integer);
                        break;

                case TOKEN_SHORTCUT:
                        right = parse_shortcut(param.token);
                        break;

                case TOKEN_VARIABLE:
                        struct Variable variable = parse_variable(param);
                        
                        right = malloc(sizeof(*right));

                        if (right != NULL) {
                                right->type = LAMBDA_VARIABLE;
                                right->variable = variable;
                        }

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
                        printf(ANSI_RED "Syntax error. Invalid operator.\n" ANSI_RESET);
                        lambda_free(left);
                        return NULL;

                case TOKEN_INVALID:
                        printf(ANSI_RED "Syntax error. Invalid character.\n" ANSI_RESET);
                        lambda_free(left);
                        return NULL;

                default:
                        right = NULL;
                        break;
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
                printf(ANSI_RED "Syntax error. Expected variable token.\n" ANSI_RESET);
                return NULL;
        }

        struct Variable variable = parse_variable(param);
        
        Lambda *lambda = malloc(sizeof(*lambda));

        if (lambda == NULL)
                return NULL;

        lambda->type = LAMBDA_ABSTRACTION;
        lambda->abstraction.binding = variable;

        get_token(param.token, param.str);

        if (param.token->type != TOKEN_DOT) {
                printf(ANSI_RED "Syntax error. Expected dot operator.\n" ANSI_RESET);
                free(lambda);
                return NULL;
        }

        param.bind = false;

        Lambda *body = parse_expression(param);

        if (body == NULL) {
                free(lambda);
                return NULL;
        }

        lambda->abstraction.body = body;

        return lambda;
}

Lambda *parse_shortcut(struct Token *token)
{
        Lambda *lambda = malloc(sizeof(*lambda));

        if (lambda == NULL)
                return NULL;

        lambda->type = LAMBDA_SHORTCUT;

        char *shortcut = duplicate_token_string(token);

        if (shortcut == NULL) {
                free(lambda);
                return NULL;
        }

        lambda->shortcut = shortcut;

        return lambda;
}

struct Variable parse_variable(struct ParseParam param)
{
        struct Variable variable;

        variable.letter = *param.token->start;
        variable.subscript = -1;

        if (!isdigit(**param.str)) {
                // No subscript
                return variable;
        }

        // Parse subscript
        get_token(param.token, param.str);

        int integer = atoi(*param.str);
        variable.subscript = integer;

        return variable;
}

Lambda *parse_binding(Lambda *left, struct ParseParam param)
{
        if (left == NULL)
                return NULL;

        if (!param.bind || left->type != LAMBDA_SHORTCUT) {
                printf(ANSI_RED "Syntax error: invalid assignment operator.\n" ANSI_RESET);
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

        char *shortcut = left->shortcut;

        left->type = LAMBDA_BIND;
        left->bind.shortcut= shortcut;
        left->bind.term = right;

        return left;
}

Lambda *parse_end(Lambda *lambda, bool parenthesis)
{
        if (parenthesis) {
                printf(ANSI_RED "Syntax error. Missing right parenthesis.\n" ANSI_RESET);
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

        if (isdigit(**c)) {
                token->type = TOKEN_NUMERAL;
                token->start = *c;

                while (isdigit(**c))
                        (*c)++;
        } else if (isupper(**c)) {
                token->type = TOKEN_SHORTCUT;
                token->start = *c;

                while (isupper(**c) || **c == '_')
                        (*c)++;
        } else if (islower(**c)) {
                token->type = TOKEN_VARIABLE;
                token->start = *c;

                (*c)++;
        }

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

Lambda *generate_church_numeral(int integer)
{
        struct Variable var_f = {
                .letter = 'f',
                .subscript = -1
        };

        struct Variable var_x = {
                .letter = 'x',
                .subscript = -1
        };

        if (integer < 0)
                return NULL;

        Lambda *numeral = malloc(sizeof(*numeral));

        if (numeral == NULL)
                return NULL;

        numeral->type = LAMBDA_ABSTRACTION;
        numeral->abstraction.binding = var_f;
        
        Lambda *inner_abstraction = malloc(sizeof(*inner_abstraction));
        
        numeral->abstraction.body = inner_abstraction;

        if (inner_abstraction == NULL) {
                free(numeral);
                return NULL;
        }

        inner_abstraction->type = LAMBDA_ABSTRACTION;
        inner_abstraction->abstraction.binding = var_x;

        Lambda *right;

        // Particular case without function application 0=\f.\x.x
        if (integer == 0) {
                right = malloc(sizeof(*right));

                inner_abstraction->abstraction.body = right;

                if (right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                right->type = LAMBDA_VARIABLE;
                right->variable = var_x;

                return numeral;
        }

        // Generate a chain of applications of the form \f.\x.f(f(...(fx)...))
        Lambda *outer_application = malloc(sizeof(*outer_application));
        inner_abstraction->abstraction.body = outer_application;

        if (outer_application == NULL){
                lambda_free(numeral);
                return NULL;
        }

        Lambda *application = outer_application;

        for (int k = 0; k < integer; k++) {
                application->type = LAMBDA_APPLICATION;

                Lambda *left = malloc(sizeof(*left));
                right = malloc(sizeof(*right));

                application->application.left = left;
                application->application.right = right;

                if (left == NULL || right == NULL) {
                        lambda_free(numeral);
                        return NULL;
                }

                left->type = LAMBDA_VARIABLE;
                left->variable = var_f;

                if (k + 1 == integer) {
                        right->type = LAMBDA_VARIABLE;
                        right->variable = var_x;

                        application->application.left = left;
                        application->application.right = right;
                } else {
                        application->application.left = left;
                        application->application.right = right;

                        application = right;
                }
        }

        return numeral;
}