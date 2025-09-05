#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "commands.h"

static bool strneq_space(const char *left, const char *right, size_t length);

static void command_help();
static Mode command_reduce(const char *str, Mode mode);
static Mode command_unbind(const char *str, Mode mode, HashTable *table);
static Mode command_call(const char *str, Mode mode);

Mode parse_command(const char *str, Mode mode, HashTable *table)
{
        if (strcmp(str, ":exit") == 0) {
                printf(" -> Exiting...\n");
                return MODE_EXIT;
        }

        if (strcmp(str, ":help") == 0) {
                command_help();
                return mode;
        }

        if (strcmp(str, ":binds") == 0) {
                hashtable_print(table);
                return mode;
        }

        if (strneq_space(str, ":reduce", 8))
                return command_reduce(str, mode);

        if (strneq_space(str, ":unbind", 8))
                return command_unbind(str, mode, table);

        if (strneq_space(str, ":call", 6))
                return command_call(str, mode);

        printf(
                "Invalid command.\n"
                "Type \":help\" for more information.\n"
        );

        return mode;
}

bool strneq_space(const char *left, const char *right, size_t length)
{
        int index = strncmp(left, right, length - 1);

        if (index != 0)
                return false;

        return isspace(left[length - 1]) || left[length - 1] == '\0';
}

void command_help()
{
        printf(
                " - Syntax\n"
                "\tThe lambda calculus is a formal language for defining function application and abstraction.\n"
                "\tA lambda calculus term is identified by a string produced by the following grammar.\n"
                "\tParentheses are part of the language, indicating the precedence of operations.\n"
                "\n"
                "\t\texpression -> variable\n"
                "\t\t            | abstraction\n"
                "\t\t            | application\n"
                "\t\t            | (expression)\n"
                "\n"
                "\t\tabstraction -> λ variable . expression\n"
                "\n"
                "\t\tapplication -> [variable | (expression) | application] [variable | (expression)]\n"
                "\n"
                "\tIn this interpreter, the backslash (\\) is used to represent the lambda operator.\n"
                "\tAlso, a variable is represented by a string of letters, digits and undescore (_).\n"
                "\n"
                "\t\tvariable -> identifier\n"
                "\n"
                "\t\tidentifier -> [a-zA-Z0-9_]+\n"
                "\n"
                "\tThis interpreter also permits defining enviroment variables using the following syntax.\n"
                "\n"
                "\t\tbind -> identifier = expression\n"
                "\n"
                " - Commands\n"
                "\t:binds -> Exhibits the user-defined enviroment free variables.\n"
                "\n"
                "\t:call <mode>   -> Defines the reduction strategy.\n"
                "\t      by name  -> Evaluates the arguments multiple times. Garanteed to terminate if possible.\n"
                "\t      by value -> Evaluates the arguments first. Faster, but not guaranteed to terminate, even if possible.\n"
                "\n"
                "\t:exit -> Terminates the program.\n"
                "\n"
                "\t:help -> Prints this message.\n"
                "\n"
                "\t:reduce <mode>  -> Defines the mode of reduction.\n"
                "\t        none    -> Signalizes to not perform reductions.\n"
                "\t        normal  -> Indicates to reduce without exhibiting steps.\n"
                "\t        verbose -> Indicates to reduce, printing each step.\n"
                "\n"
                "\t:unbind <bind> -> Removes a bind from the enviroment.\n"
        );
}

Mode command_reduce(const char *str, Mode mode)
{
        const char *p = &str[7];

        while (isspace(*p))
                p++;

        if (strcmp(p, "none") == 0) {
                printf(" -> Reduction disabled.\n");
                return 0;
        }

        if (strcmp(p, "normal") == 0) {
                printf(" -> Reducing for normal form.\n");
                return MODE_REDUCE;
        }

        if (strcmp(p, "verbose") == 0) {
                printf(" -> Reducing for normal form step by step.\n");
                return MODE_REDUCE | MODE_VERBOSE;
        }

        printf(
                ":reduce [none | normal | verbose]\n"
                "Type \":help\" for more information.\n"
        );

        return mode;
}

Mode command_unbind(const char *str, Mode mode, HashTable *table)
{
        const char *p = &str[7];

        while (isspace(*p))
                p++;

        if (*p == '\0') {
                printf(
                        ":unbind <bind>\n"
                        "Type \":help\" for more information.\n"
                );

                return mode;
        }

        bool success = hashtable_delete(table, p);

        if (success)
                printf(" -> Bind \"%s\" removed successfully.\n", p);
        else
                printf("-> No \"%s\" bind found.\n", p);

        return mode;
}

Mode command_call(const char *str, Mode mode)
{
        const char *p = &str[5];

        while (isspace(*p))
                p++;

        if (strcmp(p, "by name") == 0) {
                printf(" -> Reduction strategy set to call by name.\n");
                return mode & (MODE_REDUCE | MODE_VERBOSE);
        }

        if (strcmp(p, "by value") == 0) {
                printf(" -> Reduction strategy set to call by value.\n");
                return mode | MODE_CALL_BY_VALUE;

        }

        printf(
                ":call [by name | by value]\n"
                "Type \":help\" for more information.\n"
        );

        return mode;
}