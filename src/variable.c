#include "variable.h"

bool variable_compare(struct Variable left, struct Variable right)
{
        if (left.letter != right.letter)
                return false;

        return left.subscript == right.subscript;
}