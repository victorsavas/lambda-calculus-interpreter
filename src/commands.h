#pragma once

#include "hashtable.h"

typedef enum Mode {
        MODE_EXIT               = 1,
        MODE_REDUCE             = 2,
        MODE_VERBOSE            = 4,
        MODE_CALL_BY_VALUE      = 8
} Mode;

Mode parse_command(const char *str, Mode mode, HashTable *table);