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

static bool reduction_wrapper(Lambda *lambda, Mode mode, unsigned iterations);
static char *fgets_wrapper(char *buffer, size_t buf_size, FILE *file);  

int main()
{
        HashTable *table = hashtable_init();
        char buffer[BUF_LEN];

        Mode mode = 0;

        while (!(mode & MODE_EXIT)) {
                fgets_wrapper(buffer, BUF_LEN, stdin);
                
                if (buffer[0] == ':') {
                        mode = parse_command(buffer, mode, table);
                        continue;
                }

                Lambda *lambda;
                lambda = lambda_parse(buffer);

                if (lambda == NULL)
                        continue;

                bool valid_term = replace_shortcuts(lambda, table);

                if (!valid_term) {
                        printf("Error. Undefined free variable.\n");
                        lambda_free(lambda);
                        continue;
                }
                
                reduction_wrapper(lambda, mode, 1000);

                printf(" -> ");
                lambda_print(lambda);
                printf("\n");

                if (!hashtable_insert(table, lambda))
                        lambda_free(lambda);
        }

        hashtable_free(table);

        return 0;
}

bool reduction_wrapper(Lambda *lambda, Mode mode, unsigned iterations)
{
        lambda_reduce(lambda, mode, iterations);
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