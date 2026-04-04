#pragma once

#include "hashtable.h"

typedef enum RedStrat {
        STRAT_NORMAL,
        STRAT_APPLICATIVE,
        STRAT_CALL_BY_NAME,
        STRAT_CALL_BY_VALUE
} RedStrat;

struct Mode {
        bool exit;
        bool reduction_enabled;
        bool verbose;
        RedStrat strat;
        int depth;
};

void hello_message();
void parse_command(char *str, HashTable *table, struct Mode *mode);