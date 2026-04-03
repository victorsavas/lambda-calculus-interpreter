#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ansi_escape.h"
#include "commands.h"

const char space[] = " \t\n\v\f\r";

static void command_help();
static void command_reduce(Mode *mode, int *iterations);
static void command_remove(HashTable *table);
static int reduce_i();

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

void parse_command(char *str, HashTable *table, Mode *mode, int *iterations)
{
        if (str == NULL)
                return;

        char *token = strtok(str, space);

        if (strcmp(token, ":exit") == 0)
                *mode = MODE_EXIT;
        else if (strcmp(token, ":help") == 0)
                command_help();
        else if (strcmp(token, ":entries") == 0)
                hashtable_print(table);
        else if (strcmp(token, ":remove") == 0)
                command_remove(table);
        else if (strcmp(token, ":reduce") == 0)
                command_reduce(mode, iterations);
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
                "  :entries                       Prints out all the shortcuts stored.\n"
                "  :exit                          Terminates the program.\n"
                "  :help                          Displays this message.\n"
                "  :remove [ENTRY]                Deletes one term from the table of entries.\n"
                "  :reduce [OPTION]... [MODE]     Configures reduction iterations.\n"
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
                "The interpreter has three reduction modes:\n"
                "  disable: the interpreter simply parses expressions (default);\n"
                "  normal: the interpreter parses and then reduces expressions;\n"
                "  verbose: the interpreter parses and reduces expressions, showing all steps.\n"
                "To change reduction mode, use \":reduce [disable | normal | verbose]\".\n"
                "\n"
                "By default, the interpreter has a limit of 1000 steps of reduction.\n"
                "Its possible to change that limit using \":reduce -i STEPS\".\n"
                "\n"
                "Furthermore, its default reduction strategy is leftmost first.\n"
                "To change to rightmost first, use \":reduce rightmost\".\n"
        );
}

void command_reduce(Mode *mode, int *iterations)
{
        const char *space = " \t\n\v\f\r";
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(":reduce [rightmost | leftmost] [-i STEPS] [disable | normal | verbose]"
                        "\n");
                return;
        }

        Mode old_mode = *mode;
        int old_iterations = *iterations;

        Mode rightmost = *mode & MODE_RIGHTMOST;
        *mode ^= rightmost;

        while (token != NULL) {
                if (strcmp(token, "disable") == 0) {
                        *mode = MODE_DISABLE;
                        printf(ANSI_BLUE "Reduction disabled.\n" ANSI_RESET);
                } else if (strcmp(token, "normal") == 0) {
                        *mode = MODE_REDUCE;
                        printf(ANSI_BLUE "Reducing for normal form.\n" ANSI_RESET);
                } else if (strcmp(token, "verbose") == 0) {
                        *mode = MODE_VERBOSE;
                        printf(ANSI_BLUE "Reducing showing steps.\n" ANSI_RESET);
                } else if (strcmp(token, "leftmost") == 0)
                        rightmost = 0;
                else if (strcmp(token, "rightmost") == 0)
                        rightmost = MODE_RIGHTMOST;
                else if (strcmp(token, "-i") == 0) {
                        int steps = reduce_i();

                        if (steps <= 0)
                                goto abort;

                        *iterations = steps;
                }

                token = strtok(NULL, space);
        }

        *mode |= rightmost;

        return;

        abort:

        *mode = old_mode;
        *iterations = old_iterations;
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

int reduce_i()
{
        char *token = strtok(NULL, space);

        if (token == NULL) {
                printf(
                        ANSI_RED
                        "Syntax error. Expected parameter.\n"
                        ANSI_RESET
                        ":reduce -i [STEPS]\n"
                );
                return 0;
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
                return 0;
        }

        int steps = atoi(token);

        if (steps <= 0) {
                printf(
                        ANSI_RED
                        "Syntax error. Reduction steps number \"steps\" must be "
                        "positive.\n"
                        ANSI_RESET
                        ":reduce -i [STEPS]\n"
                );
                return 0;
        }

        return steps;
}