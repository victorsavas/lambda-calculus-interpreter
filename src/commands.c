#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escapes.h"
#include "commands.h"

const char space[] = " \t\n\v\f\r";

static void command_help();
static void command_reduce();
static void command_remove(HashTable *table);

static void reduce_s(struct Mode *new_mode, bool strat_parse);
static void reduce_i(struct Mode *new_mode, bool steps_parse);
static void reduce_v(struct Mode *new_mode, bool verbose_parse);
static void reduce_enable(struct Mode *new_mode, char *token, bool enable_parse);

void hello_message()
{
        printf(
                "Lambda Calculus (λ-calculus) abstraction and application interpreter.\n"
                "Made by victorsavas at "
                             ANSI_BLUE "https://github.com/victorsavas/lambda-calculus.\n"
                ANSI_RESET
                "Type \":help\" for more information.\n"
        );
}

void parse_command(char *str, HashTable *table)
{
        if (str == NULL)
                return;

        char *token = strtok(str, space);

        if (strcmp(token, ":exit") == 0)
                mode.exit = true;
        else if (strcmp(token, ":help") == 0)
                command_help();
        else if (strcmp(token, ":entries") == 0)
                hashtable_print(table);
        else if (strcmp(token, ":remove") == 0)
                command_remove(table);
        else if (strcmp(token, ":reduce") == 0)
                command_reduce();
        else
                printf(
                        "Invalid command \"%s\".\n"
                        "For more information, type \":help\".\n",
                        token
                );
}

void command_help()
{
        printf(
                "Lambda Calculus (λ-calculus) abstraction and application interpreter.\n"
                "Made by victorsavas at "
                             ANSI_BLUE "https://github.com/victorsavas/lambda-calculus.\n"
                ANSI_RESET "\n"
                "Command list:\n"
                "  :entries                             Prints out all the shortcuts stored.\n"
                "  :exit                                Terminates the program.\n"
                "  :help                                Displays this message.\n"
                "  :remove [ENTRY]                      Deletes one term from the table of entries.\n"
                "  :reduce [OPTION]... [ENABLE]         Sets up reduction configurations.\n"
                "\n"
                "Syntax:\n"
                "  expression -> variable\n"
                "              | abstraction\n"
                "              | application\n"
                "  variable -> a-z [0-9]*\n"                
                "  abstraction -> \\ variable . expression\n"
                "  application -> expression expression\n"
                "\n"
                "To resolve the ambiguity of this syntax, parenthesis are used.\n"
                "If parenthesis are ommited, the following rules apply:\n"
                "  1. Application has higher precedence than abstraction;\n"
                "  2. Application is left-associative (MNP is equivalent to (MN)P);\n"
                "  3. Abstraction is right-associative (\\x.\\y.M is equivalent to \\x.(\\y.M)).\n"
                "\n"
                "Particularly, these rules entail that an abstraction extends as far to the right "
                "as possible.\n"
                "\n"
                "Within this interpreter, it is possible to create shortcuts to expressions.\n"
                "  shortcut -> identifier = expression\n"
                "  identifier -> [A-Z] [A-Z_]*\n"
                "\n"
                "For convenience, the interpreter is initialized with some relevant shortcuts.\n"
                "To see them, use the command \":entries\".\n"
                "It is possible to overwrite an entries and to remove them using the \":remove\""
                " command.\n"
                "\n"
                "Furthermore, the interpreter also parses integers as Church numerals.\n"
                "For example, typing \"2\" produces the corresponding Church numeral \"\\f.\\x.f(f"
                "x)\".\n"
                "\n"
                "Reduction:\n"
                "To enable reduction, type \":reduce on\". Likewise, type \":reduce off\" to"
                " disable it.\n"
                "\n"
                "This interpreter supports two reduction strategies:\n"
                "  Normal order (:reduce -s normal)\n"
                "  Applicative order (:reduce -s applicative)\n"
                "\n"
                "To specify recursion max depth, write \":reduce -i [DEPTH]. The default depth is 1000.\n"
                "To print each reduction step, write \":reduce -v\".\n"
        );
}

void command_reduce()
{
        bool strat_parse = false;
        bool steps_parse = false;
        bool verbose_parse = false;
        bool enable_parse = false;

        const char *space = " \t\n\v\f\r";
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(
                        ":reduce [-s ORDER] [-i STEPS] [-v] ENABLE\n"
                        "ORDER: normal, applicative\n"
                        "STEPS: <integer>\n"
                        "ENABLE: enabled, disabled\n"
                );

                return;
        }
        
        struct Mode new_mode = mode;
        new_mode.verbose = false;

        while (token != NULL) {
                if (strcmp(token, "-s") == 0) {
                        reduce_s(&new_mode, strat_parse);
                        strat_parse = true;
                } else if (strcmp(token, "-i") == 0) {
                        reduce_i(&new_mode, steps_parse);
                        steps_parse = true;
                } else if (strcmp(token, "-v") == 0) {
                        reduce_v(&new_mode, verbose_parse);
                        verbose_parse = true;
                } else {
                        reduce_enable(&new_mode, token, enable_parse);
                        enable_parse = true;
                }

                if (new_mode.exit)
                        return;

                token = strtok(NULL, space);
        }

        mode = new_mode;
}

void command_remove(HashTable *table)
{
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(":remove [ENTRY]\n");
                return;
        }

        bool success = hashtable_delete(table, token);

        if (success)
                printf("Entry \"%s\" removed successfully.\n", token);
        else
                printf("No \"%s\" entry found.\n", token);
}

void reduce_s(struct Mode *new_mode, bool strat_parse)
{
        if (strat_parse) {
                printf(
                        ANSI_RED
                        "Error. Duplicate \"-s\" flag.\n"
                        ANSI_RESET
                );

                new_mode->exit = true;
                return;
        }

        const char *strat[] = {
                "normal",
                "applicative",
        };

        const RedStrat strat_code[] ={
                STRAT_NORMAL,
                STRAT_APPLICATIVE,
        };

        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(
                        ANSI_RED
                        "Syntax error. Expected argument after -s flag.\n"
                        ANSI_RESET
                        ":reduce -s [normal | applicative]\n"
                );

                new_mode->exit = true;
                return;
        }

        for (int i = 0; i < 2; i++)
                if (strcmp(token, strat[i]) == 0) {
                        new_mode->strat = strat_code[i];
                        return;
                }

        printf(
                ANSI_RED
                "Syntax error. Undefined reduction strategy \"-s %s\".\n"
                ANSI_RESET
                ":reduce -s [normal | applicative]\n",
                token
        );

        new_mode->exit = true;
}

void reduce_i(struct Mode *new_mode, bool steps_parse)
{
        if (steps_parse) {
                printf(
                        ANSI_RED
                        "Error. Duplicate \"-i\" flag.\n"
                        ANSI_RESET
                );

                new_mode->exit = true;
                return;
        }

        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(
                        ANSI_RED
                        "Syntax error. Expected parameter.\n"
                        ANSI_RESET
                        ":reduce -i [STEPS]\n"
                );

                new_mode->exit = true;
                return;
        }

        if (!isdigit(token[0])) {
                printf(
                        ANSI_RED
                        "Syntax error. Invalid parameter \"%s\" (expected positive"
                        " integer value).\n"
                        ANSI_RESET
                        ":reduce -i [STEPS]\n",
                        token
                );

                new_mode->exit = true;
                return;
        }

        int depth = atoi(token);

        if (depth <= 0) {
                printf(
                        ANSI_RED
                        "Syntax error. Reduction steps number \"steps\" must be "
                        "positive.\n"
                        ANSI_RESET
                        ":reduce -i [STEPS]\n"
                );
                
                new_mode->exit = true;
                return;
        }

        new_mode->depth = depth;
}

void reduce_v(struct Mode *new_mode, bool verbose_parse)
{
        if (verbose_parse) {
                printf(
                        ANSI_RED
                        "Error. Duplicate \"-v\" flag.\n"
                        ANSI_RESET
                );

                new_mode->exit = true;
                return;
        }

        new_mode->verbose = true;
}

void reduce_enable(struct Mode *new_mode, char *token, bool enable_parse)
{
        if (enable_parse) {
                printf(
                        ANSI_RED
                        "Error. Invalid token \"%s\".\n"
                        ANSI_RESET
                        ":reduce [OPTION]... [on | off]\n",
                        token
                );

                new_mode->exit = true;
                return;
        }

        const char *enable[] = {
                "on",
                "off"
        };

        if (strcmp(token, enable[0]) == 0) {
                new_mode->reduction_enabled = true;
                return;
        }
        
        if (strcmp(token, enable[1]) == 0) {
                new_mode->reduction_enabled = false;
                return;
        }
        
        printf(
                ANSI_RED
                "Syntax error. Invalid token \"%s\".\n"
                ANSI_RESET
                ":reduce [OPTION]... [on | off]\n",
                token
        );

        new_mode->exit = true;
}