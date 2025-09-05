#pragma once

#include <stdbool.h>

#include "parsing.h"
#include "hashtable.h"

bool lambda_replace_free_variables(Lambda *lambda, HashTable *table);