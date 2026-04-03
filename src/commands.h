#pragma once

#include "hashtable.h"

typedef enum Mode {
        MODE_EXIT               = 0,
        MODE_DISABLE            = 1,
        MODE_REDUCE             = 2,
        MODE_VERBOSE            = 4,
        MODE_RIGHTMOST          = 8
} Mode;

void hello_message();
void parse_command(char *str, HashTable *table, Mode *mode, int *iterations);