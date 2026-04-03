#include "variable.h"

#include <stdio.h>

void variable_print(struct Variable variable)
{
        if (variable.subscript < 0)
                printf("%c", variable.letter);
        else
                printf("%c%d", variable.letter, variable.subscript);
}

bool variable_compare(struct Variable left, struct Variable right)
{
        if (left.letter != right.letter)
                return false;

        return left.subscript == right.subscript;
}

bool variable_search(const void *left, const void *right)
{
        struct Variable var_left = *(const struct Variable *)left;
        struct Variable var_right = *(const struct Variable *)right;

        return variable_compare(var_left, var_right);
}