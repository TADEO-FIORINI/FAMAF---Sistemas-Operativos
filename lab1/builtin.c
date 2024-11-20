#include "builtin.h"
#include "command.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tests/syscall_mock.h"

bool builtin_is_internal(scommand cmd) {
  assert(cmd != NULL);
  bool is_internal;
  char *cmd_name = scommand_front(cmd);
  if (strcmp(cmd_name, "cd") == 0 || strcmp(cmd_name, "help") == 0 ||
      strcmp(cmd_name, "exit") == 0) {
    is_internal = true;
  } else {
    is_internal = false;
  }
  return is_internal;
}

bool builtin_alone(pipeline p) {
  assert(p != NULL);
  return (pipeline_length(p) == 1 && builtin_is_internal(pipeline_front(p)));
}

void builtin_run(scommand cmd) {
  assert(builtin_is_internal(cmd));
  char *cmd_type = scommand_front(cmd);
  if (cmd_type != NULL) {
    if (strcmp(cmd_type, "cd") == 0) {
      if (scommand_length(cmd)>1) {
        scommand_pop_front(cmd);
        char *filename = scommand_front(cmd); 
        int sucess = chdir (filename);
        if (sucess != 0){
          printf ("No existe el directorio \n");
        }
        }      
      else {
        char *username = getenv("HOME");
        if (username != NULL) {
          chdir(username); 
        } 
      }
    } else if (strcmp(cmd_type, "help") == 0)
    {
      printf("shellname:YGTB\n");
      printf("autores: Gregorio Vilardo, Tadeo Fiorini, Braian Rodriguez, "
             "Yamil Ali\n");
      printf("Comandos internos -> cd: Cambia de directorio\n help:muestra "
             "informacion relevante\n exit: sale de la shell\n");
    } else if (strcmp(cmd_type, "exit") == 0)
    // comando exit que termina la sesion en el shell
    {
      exit(EXIT_SUCCESS);
    }
  }
}
