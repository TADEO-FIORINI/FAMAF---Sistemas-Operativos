#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "builtin.h"
#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"

#include <fcntl.h>    // open
#include <sys/wait.h> // wait
#include <unistd.h>   // libreria con chdir

#include "tests/syscall_mock.h"

#define ERROR -1
#define CHILD 0
#define NO_FD -2

// Obtiene los descriptores de archivo de redirecion y apunta los pointers a
// ellos
void get_fd_redir(scommand scmd, int *fd_in, int *fd_out) {
  char *redir_in, *redir_out;
  redir_in = scommand_get_redir_in(scmd);
  redir_out = scommand_get_redir_out(scmd);
  if (redir_in != NULL) {
    printf("%s\n", redir_in);
    *fd_in = open(redir_in, O_RDONLY, S_IRUSR);
    if (*fd_in == ERROR) {
      *fd_in = NO_FD;
      perror("Error reading file");
      exit(EXIT_FAILURE);
    }

  } else {
    *fd_in = NO_FD;
  }

  if (redir_out != NULL) {
    *fd_out = open(redir_out, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (*fd_out == ERROR) {
      *fd_out = NO_FD;
      perror("Error writting file");
      exit(EXIT_FAILURE);
    }

  } else {
    *fd_out = NO_FD;
  }
}

// Obtiene el comando como un array de char*
char **get_argv(scommand scmd) {
  assert(scmd != NULL);

  unsigned int scmd_len = scommand_length(scmd);
  // doble pointer para un array "bidimensional" de caracteres
  char **argv = (char **)malloc((sizeof(char *) * (scmd_len + 1)));

  if (argv != NULL) {
    for (unsigned int i = 0; i < scmd_len; i++) {
      // duplicamos el string ya que pop lo destruye
      char *arg = strdup(scommand_front(scmd));
      scommand_pop_front(scmd);
      argv[i] = arg;
    }
    argv[scmd_len] = NULL;
  }

  return argv;
}

// redirigimos todos los files descriptors que se vayan a utilizar
void dup_and_close(int fd_redir_in, int fd_redir_out, int *fd_in, int *fd_out) {
  if (fd_redir_out != NO_FD) {
    dup2(fd_redir_out, STDOUT_FILENO);
    close(fd_redir_out);
  }
  if (*fd_out != NO_FD) {
    dup2(*fd_out, STDOUT_FILENO);
    close(*fd_out);
  }
  if (fd_redir_in != NO_FD) {
    dup2(fd_redir_in, STDIN_FILENO);
    close(fd_redir_in);
  }
  if (*fd_in != NO_FD) {
    dup2(*fd_in, STDIN_FILENO);
    close(*fd_in);
  }
}

void execute_scmd(scommand scmd, bool is_pipe, bool is_wait, int *fd_in,
                  int *fd_out, unsigned int *wait_counter) {
  assert(scmd != NULL);
  int pipefd[2] = {NO_FD, NO_FD};
  pid_t pid;
  int fd_redir_in, fd_redir_out;
  char **argv = get_argv(scmd);

  if (is_pipe) {
    if (pipe(pipefd) == ERROR) {
      perror("Error en el pipe");
      exit(EXIT_FAILURE);
    }
  }
  switch (pid = fork()) {

  case ERROR:
    free(argv);
    perror("Error en el fork\n");
    exit(EXIT_FAILURE);
    break;

  case CHILD:
    get_fd_redir(scmd, &fd_redir_in, &fd_redir_out);
    if (is_pipe) {
      // Cerramos el de lectura que no vamos a usar
      close(pipefd[0]);
      dup2(pipefd[1], STDOUT_FILENO); // Redirigimos y luego cerramos
      close(pipefd[1]);
    }

    dup_and_close(fd_redir_in, fd_redir_out, fd_in, fd_out);

    execvp(argv[0], argv);
    perror("Error en ejecucion");
    exit(EXIT_FAILURE);
    break;
  default:
    // liberamos argv ya que no lo usamos
    free(argv);
    // En el padre vemos si es pipe, en ese caso apuntamos fd_in al pipe de
    // lectura
    if (is_pipe) {
      close(pipefd[1]);
      *fd_in = pipefd[0];
      // En el caso de que no estemos en un pipe y exista un fd_in entonces
      // quiere decir que venimos de un pipe por lo que hay que cerrarlo
    } else if (*fd_in != NO_FD) {
      close(*fd_in);
    }
    // Contamos cuantos wait hubo para esperarlos en el padre
    *wait_counter += 1;
  }
}

void execute_pipeline(pipeline apipe) {
  assert(apipe != NULL);
  if (!pipeline_is_empty(apipe)) {
    scommand scmd;
    int fd_in = NO_FD, fd_out = NO_FD;
    bool is_pipe = false;
    bool is_wait = true;
    unsigned int wait_counter = 0;

    if (builtin_alone(apipe)) {
      scmd = pipeline_front(apipe);
      builtin_run(scmd);
      pipeline_pop_front(apipe);
    } else {
      // Recorremos todo el pipeline
      while (!pipeline_is_empty(apipe)) {
        scmd = pipeline_front(apipe);
        is_wait = pipeline_get_wait(apipe);
        // En caso de que el pipe tenga mas de un elemento entonces llamamos a
        // pipe()
        is_pipe = (pipeline_length(apipe) == 1) ? false : true;
        execute_scmd(scmd, is_pipe, is_wait, &fd_in, &fd_out, &wait_counter);
        // Lo quitamos de la pipe una vez usado, ya que este procedimiento lo destruye
        // asi no perdemos el pointer
        pipeline_pop_front(apipe);
        // Si es wait y si !is_pipe espera a los hijos
        // El !is_pipe esta porque si 'is_pipe == true' entonces hubo dos fork()
        // y estariamos en uno de sus hijos
        if (is_wait) {
          if (!is_pipe) {
            for (unsigned int i = 0; i < wait_counter; i++) {
              if (wait(NULL) == ERROR) {
                perror("Error wait");
              }
            }
          }
        }
      }
    }
  }
}
