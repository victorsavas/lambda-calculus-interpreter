#pragma once

#include "parsing.h"

typedef struct HashTable HashTable;

HashTable *hashtable_init();
void hashtable_free(HashTable *table);

Lambda *hashtable_insert(HashTable *table, Lambda *lambda);
Lambda *hashtable_search(HashTable *table, const char *key);
Lambda *hashtable_delete(HashTable *table, const char *key);

void hashtable_print(HashTable *table);