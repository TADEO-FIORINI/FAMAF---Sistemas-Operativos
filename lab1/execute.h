/* Ejecuta comandos simples y pipelines.
 * No toca ningún comando interno.
 */

#ifndef EXECUTE_H
#define EXECUTE_H

#include "command.h"

void execute_scmd(scommand scmd, bool is_pipe, bool is_wait, int *fd_in,
                  int *fd_out, unsigned int *wait_counter);
/* Ejecuta un scmd, en el cual cambia los pointers a los fd en cada caso
 * Tambien actualiza el wait_counter para saber cuantos waits se necesitaran en
 * execute_pipeline Requires: scmd != NULL
 */

void get_fd_redir(scommand scmd, int *fd_in, int *fd_out);
/* Obtiene los descriptores de archivos para cada redir */

void dup_and_close(int fd_redir_in, int fd_redir_out, int *fd_in, int *fd_out);
/* Redirige los STD correspondientes a cada descriptor*/

char **get_argv(scommand scmd);
/* Pasa el scmd a un array de char* 
 * Requires: scmd != NULL
 * */

void execute_pipeline(pipeline apipe);
/*
 * Ejecuta un pipeline, identificando comandos internos, forkeando, y
 *   redirigiendo la entrada y salida. puede modificar `apipe' en el proceso
 *   de ejecución.
 *   apipe: pipeline a ejecutar
 * Requires: apipe!=NULL
 */

#endif /* EXECUTE_H */
