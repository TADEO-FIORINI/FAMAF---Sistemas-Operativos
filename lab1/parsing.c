#include <stdbool.h>
#include <stdlib.h>

#include "command.h"
#include "parser.h"
#include "parsing.h"

static scommand parse_scommand(Parser p) {
  /* Devuelve NULL cuando hay un error de parseo */
  scommand command = scommand_new();
  arg_kind_t arg_type;
  char *arg;
  while ((arg = parser_next_argument(p, &arg_type)) != NULL) {
    if (arg_type == ARG_NORMAL) {
      scommand_push_back(command, arg);
    } else if (arg_type == ARG_INPUT) {
      scommand_set_redir_in(command, arg);
    } else if (arg_type == ARG_OUTPUT) {
      scommand_set_redir_out(command, arg);
    }
  }
  if (scommand_is_empty(command)) {
    command = scommand_destroy(command);
  }
  return command;
}

pipeline parse_pipeline(Parser p) {
  assert(p != NULL && !parser_at_eof(p));
  pipeline result = pipeline_new();
  scommand cmd = NULL;
  bool garbage = false;
  bool error = false, another_pipe = true, is_background = false;
  parser_skip_blanks(p);
  cmd = parse_scommand(p);
  error = (cmd == NULL); /* Comando inválido al empezar */
  while (another_pipe && !error) {
    pipeline_push_back(result, cmd);
    parser_skip_blanks(p);
    parser_op_pipe(p, &another_pipe);
    cmd = parse_scommand(p);
    error = (cmd == NULL);
  }
  parser_op_background(p, &is_background);
  pipeline_set_wait(result, !is_background);
  parser_garbage(p, &garbage);
  if (pipeline_is_empty(result)) {
    result = pipeline_destroy(result);
  }
  return result;
}
