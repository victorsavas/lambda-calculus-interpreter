#pragma once

#include "hashtable.h"

typedef enum RedStrat {
        STRAT_NORMAL,
        STRAT_APPLICATIVE,
} RedStrat;

struct Mode {
        bool exit;
        bool interrupt;
        bool reduction_enabled;
        bool verbose;
        RedStrat strat;
        int depth;
};

extern struct Mode mode;

void hello_message();
void parse_command(char *str, HashTable *table);