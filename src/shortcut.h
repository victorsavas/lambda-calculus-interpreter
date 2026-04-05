#pragma once

#include <stdbool.h>

#include "ast.h"
#include "hashtable.h"

bool replace_shortcuts(Lambda *lambda, HashTable *table);