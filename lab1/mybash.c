#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

static void show_prompt(void) {
   char buf[1024]; // Un buffer para almacenar el directorio actual
    if (getcwd(buf, sizeof(buf)) != NULL) {
        printf ("%s> ",buf);
    }
    fflush (stdout);
}

int main(int argc, char *argv[]) {
    pipeline pipe;
    Parser input;
    bool quit = false;
    input = parser_new(stdin);
    while (!quit) {
        show_prompt();
        pipe = parse_pipeline(input);
        quit = parser_at_eof(input);
        if (pipe!=NULL){
          execute_pipeline(pipe);
          pipe = pipeline_destroy(pipe);
        }
    }
    parser_destroy(input); input = NULL;
    return EXIT_SUCCESS;
}
