#pragma once

#include "parsing.h"
#include "stack.h"

Stack *get_free_variables(Lambda *lambda);
Lambda *variable_capture(Lambda *application);