#include <ctype.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "hashtable.h"
#include "printing.h"
#include "shortcut.h"
#include "reduction.h"
#include "commands.h"

#define BUF_LEN 65536

struct Mode mode = {
        .exit = false,
        .interrupt = false,
        .reduction_enabled = false,
        .verbose = false,
        .depth = 1000
};

HashTable *table;

static void interrupt_handle(int signal);
static char *fgets_wrapper(char *buffer, size_t buf_size, FILE *file);  

int main()
{
        table = hashtable_init();
        char buffer[BUF_LEN];

        hello_message();

        while (!mode.exit) {
                signal(SIGINT, interrupt_handle);

                printf("λ> ");

                fgets_wrapper(buffer, BUF_LEN, stdin);
                
                if (buffer[0] == ':') {
                        parse_command(buffer, table);
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
                
                lambda_reduce(lambda);

                mode.interrupt = false;

                if (!hashtable_insert(table, lambda))
                        lambda_free(lambda);
        }

        printf("Exiting.\n");

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

void interrupt_handle(int signal)
{
        printf("\nExiting.\n");
        hashtable_free(table);

        exit(1);
}