#pragma once

#include "stdbool.h"

struct Variable {
        char letter;
        int subscript;
};

bool variable_compare(struct Variable left, struct Variable right);