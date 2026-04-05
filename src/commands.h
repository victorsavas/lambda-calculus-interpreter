#pragma once

#include "hashtable.h"

struct Mode {
        bool exit;
        bool interrupt;
        bool reduction_enabled;
        bool verbose;
        int depth;
};

extern struct Mode mode;

void hello_message();
void parse_command(char *str, HashTable *table);