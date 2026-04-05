#pragma once

#include <stdbool.h>

#include "ast.h"
#include "stack.h"

bool alpha_rename(Lambda *redex);

bool is_redex(Lambda *lambda);
Stack *get_free_variables(Lambda *lambda);