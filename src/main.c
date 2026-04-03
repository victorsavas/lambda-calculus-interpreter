#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "parsing.h"
#include "hashtable.h"
#include "printing.h"
#include "shortcut.h"
#include "reduction.h"
#include "commands.h"

#define BUF_LEN 65536

static char *fgets_wrapper(char *buffer, size_t buf_size, FILE *file);  

int main()
{
        HashTable *table = hashtable_init();
        char buffer[BUF_LEN];

        Mode mode = MODE_DISABLE;
        int iterations = 1000;

        hello_message();

        while (mode != MODE_EXIT) {
                printf("λ> ");

                fgets_wrapper(buffer, BUF_LEN, stdin);
                
                if (buffer[0] == ':') {
                        parse_command(buffer, table, &mode, &iterations);
                        continue;
                }

                Lambda *lambda;
                lambda = lambda_parse(buffer);

                if (lambda == NULL)
                        continue;

                bool valid_term = replace_shortcuts(lambda, table);

                if (!valid_term) {
                        lambda_free(lambda);
                        continue;
                }
                
                lambda_reduce(lambda, mode, iterations);

                if (!hashtable_insert(table, lambda))
                        lambda_free(lambda);
        }

        hashtable_free(table);

        return 0;
}

char *fgets_wrapper(char *buffer, size_t buf_len, FILE *file)
{
        if (buffer == NULL || buf_len == 0 || file == NULL)
                return NULL;

        buffer = fgets(buffer, buf_len, file);

        if (buffer == NULL)
                return NULL;

        char *p = buffer;

        while (*p != '\0' && *p != '\r' && *p != '\n')
                p++;

        *p = '\0';

        return buffer;
}