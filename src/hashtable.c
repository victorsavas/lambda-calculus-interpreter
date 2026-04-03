#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "ansi_escape.h"
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
        size_t entries_count;
} HashTable;

static void nodes_free(Node *node);
static Node *node_tail(Node *node);
static uint32_t hash_function(const char *key);

static void load_default_shortcuts(HashTable *table);

static int compare_entries(const void *left, const void *right);

HashTable *hashtable_init()
{
        HashTable *table = calloc(1, sizeof(*table));

        load_default_shortcuts(table);

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
                table->entries_count++;
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

        table->entries_count++;

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

        table->entries_count--;

        return lambda;
}

void hashtable_print(HashTable *table)
{
        if (table == NULL)
                return;

        size_t entries_count = table->entries_count;

        if (entries_count == 0) {
                printf("Empty.\n");
        }

        Lambda **array = malloc(sizeof(*array) * entries_count);

        if (array == NULL)
                return;

        size_t i = 0;

        for (size_t j = 0; j < BUCKET_COUNT && i < entries_count; j++) {
                Node *node = &table->buckets[j];

                if (node->value == NULL)
                        continue;

                while (node != NULL) {
                        array[i++] = node->value;
                        node = node->next;
                }
        }

        qsort(array, entries_count, sizeof(*array), compare_entries);

        for (i = 0; i < entries_count; i++) {
                printf(ANSI_BLUE "%-4d " ANSI_RESET, i + 1);
                
                char *shortcut = array[i]->bind.shortcut;
                Lambda *term = array[i]->bind.term;

                printf("%-8s " ANSI_GREEN, shortcut);

                lambda_print(term, NULL);
                printf(ANSI_RESET "\n");
        }
        
        free(array);
}

void load_default_shortcuts(HashTable *table)
{
        const char *str_shortcuts[] = {
                // Basic combinators

                "S=\\x.\\y.\\z.xz(yz)",
                "K=\\x.\\y.x",
                "I=\\x.x",
                "B=\\x.\\y.\\z.x(yz)",
                "C=\\x.\\y.\\z.xzy",

                "DUP=\\x.xx",
                "OMEGA=(\\x.xx)(\\x.xx)",

                "Y=\\f.(\\x.f(xx))(\\x.f(xx))",
                "THETA=(\\x.\\y.y(xxy))(\\x.\\y.y(xxy))",
                
                // Booleans

                "TRUE=\\f.\\x.f",
                "FALSE=\\f.\\x.x",
                "IF=\\p.\\a.\\b.pab",

                // Logic operators

                "NOT=\\p.p(\\f.\\x.x)(\\f.\\x.f)",
                
                "AND=\\p.\\q.pq(\\f.\\x.x)",
                "NAND=\\p.\\q.p(q(\\f.\\x.x)(\\f.\\x.f))(\\f.\\x.f)",
                
                "OR=\\p.\\q.p(\\f.\\x.f)q",
                "NOR=\\p.\\q.p(\\f.\\x.x)(q(\\f.\\x.x)(\\f.\\x.f))",

                "XOR=\\p.\\q.p(q(\\f.\\x.x)(\\f.\\x.f))q",
                "XNOR=\\p.\\q.pq(q(\\f.\\x.x)(\\f.\\x.f))",
                
                "IMPLY=\\p.\\q.pq(\\f.\\x.f)",
                "NIMPLY=\\p.\\q.p(q(\\f.\\x.x)(\\f.\\x.f))(\\f.\\x.x)",
                
                // Arithmetic

                "INCR=(\\p.\\q.\\f.\\x.pf(qfx))(\\f.\\x.fx)",
                "DECR=\\p.(\\p.p(\\x.\\f.\\x.x)(\\f.\\x.f))p(\\f.\\x.x)((\\p."
                     "\\f.\\x.p(\\g.\\h.h(gf))(\\u.x)(\\u.u))p)",

                "ADD=\\p.\\q.\\f.\\x.pf(qfx)",
                "MINUS=\\p.\\q.q(\\p.\\f.\\x.p(\\g.\\h.h(gf))(\\u.x)(\\u.u))p",
                "TIMES=\\p.\\q.\\f.p(qf)",

                "IS_ZERO=\\p.p(\\x.(\\f.\\x.x))(\\f.\\x.f)",
                "POW=\\p.\\q.q(\\x.\\f.\\x.x)(\\f.\\x.f)(\\f.\\x.fx)(qp)",

                "EVEN=\\n.n(\\p.p(\\f.\\x.x)(\\f.\\x.f))(\\f.\\x.f)",
                "ODD=\\n.n(\\p.p(\\f.\\x.x)(\\f.\\x.f))(\\f.\\x.x)",

                "SUM=\\f.\\n.\\y.\\x.n(\\p.\\y.y(\\y.\\x.y(p(\\y.\\x.y)yx))(\\"
                    "y.\\x.p(\\y.\\x.x)y(f(p(\\y.\\x.y))yx)))(\\y.y(\\y.\\x.x)"
                    "(\\y.\\x.x))(\\y.\\x.x)y(f(n(\\p.\\y.y(\\y.\\x.y(p(\\y.\\"
                    "x.y)yx))(\\y.\\x.p(\\y.\\x.x)y(f(p(\\y.\\x.y))yx)))(\\f.f"
                    "(\\f.\\x.x)(\\f.\\x.x))(\\y.\\x.y))yx)",

                // Quantifiers

                "EXISTS=\\p.(\\f.(\\x.f(xx))(\\x.f(xx)))(\\f.\\p.\\n.(\\p.\\q."
                       "p(\\f.\\x.f)q)(pn)(fp((\\p.\\q.\\f.\\x.pf(qfx))n(\\f."
                       "\\x.fx))))p(\\f.\\x.x)",
                
                "FORALL=\\p.(\\f.(\\x.f(xx))(\\x.f(xx)))(\\f.\\p.\\n.(\\p.\\q."
                       "pq(\\f.\\x.x))(pn)(fp((\\p.\\q.\\f.\\x.pf(qfx))n(\\f."
                       "\\x.fx))))p(\\f.\\x.x)",

                // Data structures

                "PAIR=\\x.\\y.\\f.fxy",
                "FIRST=\\p.p(\\f.\\x.f)",
                "SECOND=\\p.p(\\f.\\x.x)",

                "ROOT=\\x.\\l.\\r.\\f.\\x.fx(\\f.flr)",
                "DATUM=\\n.n(\\f.\\x.f)",

                "EMPTY=\\x.(\\f.\\x.f)",
                "NULL=\\p.p(\\x.\\y.(\\f.\\x.x))",

                "LEFT=\\n.n(\\f.\\x.x)(\\f.\\x.f)",
                "RIGHT=\\n.n(\\f.\\x.x)(\\f.\\x.x)"
        };

        const int length = sizeof(str_shortcuts) / sizeof(str_shortcuts[0]);

        for (int i = 0; i < length; i++) {
                Lambda *shortcut = lambda_parse(str_shortcuts[i]);

                if (!hashtable_insert(table, shortcut)) {
                        printf(ANSI_RED "Error at index %d: \"%s\".\n" ANSI_RESET, i, str_shortcuts[i]);
                        lambda_free(shortcut);
                }
        }
}

int compare_entries(const void *left, const void *right)
{
        Lambda *l_left = *(Lambda **)left;
        Lambda *l_right = *(Lambda **)right;

        char *str_left = l_left->shortcut;
        char *str_right = l_right->shortcut;

        return strcmp(str_left, str_right);
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