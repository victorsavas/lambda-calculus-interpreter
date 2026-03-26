#pragma once

#include <stdbool.h>

#include "parsing.h"
#include "hashtable.h"

bool replace_shortcuts(Lambda *lambda, HashTable *table);