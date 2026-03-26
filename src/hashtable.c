#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "printing.h"
#include "hashtable.h"

#define BUCKET_COUNT 256

typedef struct Node Node;

typedef struct Node {
        Lambda *value;
        Node *next;
} Node;

typedef struct HashTable {
        Node buckets[BUCKET_COUNT];
} HashTable;

static void nodes_free(Node *node);
static Node *node_tail(Node *node);
static uint32_t hash_function(const char *key);

HashTable *hashtable_init()
{
        HashTable *table = calloc(1, sizeof(*table));
        return table;
}

void hashtable_free(HashTable *table)
{
        if (table == NULL)
                return;

        for (size_t i = 0; i < BUCKET_COUNT; i++) {
                Node *bucket = &table->buckets[i];
                
                if (bucket->value != NULL) {
                        lambda_free(bucket->value);
                        nodes_free(bucket->next);
                }
        }

        free(table);
}

Lambda *hashtable_insert(HashTable *table, Lambda *lambda)
{
        if (table == NULL || lambda == NULL)
                return NULL;

        if (lambda->type != LAMBDA_BIND)
                return NULL;

        const char *insert_name = lambda->bind.shortcut;
        uint32_t hash = hash_function(insert_name);
        size_t index = hash % BUCKET_COUNT;

        Node *bucket = &table->buckets[index];

        if (bucket->value == NULL) {
                bucket->value = lambda;
                return lambda;
        }
        
        Node *node = bucket;

        while (node != NULL) {
                const char *node_name = node->value->bind.shortcut;
                if (strcmp(insert_name, node_name) == 0) {
                        lambda_free(node->value);
                        node->value = lambda;
                        return lambda;
                }
                node = node->next;
        }
        
        node = malloc(sizeof(*node));

        if (node == NULL) {
                lambda_free(lambda);
                return NULL;
        }

        node->value = lambda;
        node->next = bucket->next;

        bucket->next = node;

        return lambda;
}

Lambda *hashtable_search(HashTable *table, const char *key)
{
        if (table == NULL || key == NULL)
                return NULL;

        uint32_t hash = hash_function(key);
        size_t index = hash % BUCKET_COUNT;

        Node *node = &table->buckets[index];

        if (node->value == NULL)
                return NULL;

        while (strcmp(node->value->bind.shortcut, key) != 0) {
                node = node->next;

                if (node == NULL)
                        return NULL;
        }

        return node->value;
}

Lambda *hashtable_delete(HashTable *table, const char *key)
{
        if (table == NULL || key == NULL)
                return NULL;

        uint32_t hash = hash_function(key);
        size_t index = hash % BUCKET_COUNT;

        Node *node = &table->buckets[index];

        if (node->value == NULL)
                return NULL;

        Node *prev = NULL;

        while (strcmp(node->value->bind.shortcut, key) != 0) {
                prev = node;
                node = node->next;

                if (node == NULL)
                        return NULL;
        }

        Lambda *lambda = node->value;

        if (prev != NULL) {
                prev->next = node->next;
                free(node);
        } else {
                Node *next = node->next;

                if (next != NULL) {
                        node->value = next->value;
                        node->next = next->next;

                        free(next);
                } else {
                        node->value = NULL;
                        node->next = NULL;
                }
        }

        return lambda;
}

void hashtable_print(HashTable *table)
{
        if (table == NULL)
                return;

        for (size_t i = 0; i < BUCKET_COUNT; i++) {
                Node *node = &table->buckets[i];

                if (node->value == NULL)
                        continue;

                while (node != NULL) {
                        lambda_print(node->value);
                        printf("\n");

                        node = node->next;
                }
        }
}

uint32_t hash_function(const char *key)
{
        // DJB2 hashing

        uint32_t hash = 5381;
        int c;

        while (c = *key++)
                hash = ((hash << 5) + hash) + c;

        return hash;
}

Node *node_tail(Node *node)
{
        while (node->next != NULL)
                node = node->next;

        return node;
}

void nodes_free(Node *node)
{
        while (node != NULL) {
                Node *next = node->next;

                lambda_free(node->value);
                free(node);

                node = next;
        }
}