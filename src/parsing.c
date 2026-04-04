#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escapes.h"
#include "parsing.h"
#include "stack.h"

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
        long int integer;
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
static Lambda *parse_entry(Lambda *left, struct ParseParam param);
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

        Stack *stack = stack_init();

        if (stack == NULL) {
                fprintf(stderr, "Fatal error. Unable to initialize stack.\n");
                exit(EXIT_FAILURE);
        }

        const Lambda *top = lambda;

        while (top != NULL) {
                switch (top->type) {
                case LAMBDA_ENTRY:
                        free(top->entry);
                        stack_push(stack, top->term);
                        break;
                
                case LAMBDA_SHORTCUT:
                        free(top->shortcut);
                        break;

                case LAMBDA_VARIABLE:
                        break;

                case LAMBDA_ABSTRACTION:
                        stack_push(stack, top->body);
                        break;

                case LAMBDA_APPLICATION:
                        stack_push(stack, top->right);
                        stack_push(stack, top->left);

                        break;
                
                case LAMBDA_NUMERAL:
                        break;
                }

                free((void *)top);
                top = stack_pop(stack);
        }

        stack_free(stack);
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
                        right = malloc(sizeof(*right));

                        if (right == NULL)
                                break;

                        right->type = LAMBDA_NUMERAL;
                        right->numeral = param.token->integer;

                        break;

                case TOKEN_SHORTCUT:
                        right = parse_shortcut(param.token);
                        break;

                case TOKEN_VARIABLE:
                        struct Variable variable = parse_variable(param);
                        
                        right = malloc(sizeof(*right));

                        if (right == NULL)
                                break;
                        
                        right->type = LAMBDA_VARIABLE;
                        right->variable = variable;

                        break;
        
                case TOKEN_EQUALS:
                        return parse_entry(left, param);

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

                Lambda *application = malloc(sizeof(*application));

                if (application == NULL) {
                        lambda_free(left);
                        lambda_free(right);
                        return NULL;
                }

                application->type = LAMBDA_APPLICATION;
                application->left = left;
                application->right = right;

                left = application;

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
        
        Lambda *abstraction = malloc(sizeof(*abstraction));

        if (abstraction == NULL)
                return NULL;

        abstraction->type = LAMBDA_ABSTRACTION;
        abstraction->variable = variable;

        get_token(param.token, param.str);

        if (param.token->type != TOKEN_DOT) {
                printf(ANSI_RED "Syntax error. Expected dot operator.\n" ANSI_RESET);
                free(abstraction);
                return NULL;
        }

        param.bind = false;

        Lambda *body = parse_expression(param);

        if (body == NULL) {
                free(abstraction);
                return NULL;
        }

        abstraction->body = body;

        return abstraction;
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

Lambda *parse_entry(Lambda *left, struct ParseParam param)
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

        char *entry = left->entry;

        left->type = LAMBDA_ENTRY;
        left->entry = entry;
        left->term = right;

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
                printf(ANSI_RED "Syntax error: unexpected parenthesis.\n" ANSI_RESET);
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
                .size = 0,
                .integer = 0
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
                char *end;
                token->integer = strtol(token->start, &end, 10);
                *c = end;
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