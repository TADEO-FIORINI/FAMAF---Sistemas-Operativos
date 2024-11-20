#include "command.h"
#include <assert.h>

struct scommand_s {
  GQueue *cmd_args; // Almacena los argumentos
  char *redir_in;   // Redirecci�n de entrada
  char *redir_out;  // Redirecci�n de salida
};

struct pipeline_s {
  GQueue *scmds; // Usamos una cola para almacenar los comandos en el pipeline
  bool wait;
};

scommand scommand_new(void) {
  scommand scmd = malloc(sizeof(struct scommand_s));
  if (scmd != NULL) {
    scmd->cmd_args = g_queue_new();
    scmd->redir_in = NULL;
    scmd->redir_out = NULL;
  }
  assert(scmd != NULL && scommand_is_empty(scmd) &&
         scommand_get_redir_in(scmd) == NULL &&
         scommand_get_redir_out(scmd) == NULL);

  return scmd;
}

scommand scommand_destroy(scommand self) {
  assert(self != NULL);
  // Liberar memoria del cmd y los argumentos
  g_queue_free_full(self->cmd_args, g_free);
  self->cmd_args = NULL;
  // Liberar memoria de las redirecciones
  if (self->redir_in != NULL) {
    free(self->redir_in);
    self->redir_in = NULL;
  }
  if (self->redir_out != NULL) {
    free(self->redir_out);
    self->redir_out = NULL;
  }
  free(self); // Liberar la estructura
  self = NULL;
  assert(self == NULL);
  return self;
}

void scommand_push_back(scommand self, char *argument) {
  assert(self != NULL && argument != NULL);
  g_queue_push_tail(self->cmd_args, argument);
  // free(argument);
  // argument = NULL;
  assert(!scommand_is_empty(self));
}

void scommand_pop_front(scommand self) {
  assert(self != NULL && !scommand_is_empty(self));
  gpointer head = g_queue_pop_head(self->cmd_args);
  g_free(head); //head can be NULL
  head = NULL;
}

void scommand_set_redir_in(scommand self, char *filename) {
  assert(self != NULL);
  // destruye la redireccion in en caso de existir
  if (self->redir_in != NULL) {
    free(self->redir_in);
  }
  if (filename != NULL) {
    self->redir_in = filename;
    // free(filename);
    // filename = NULL;
  } else {
    self->redir_in = NULL;
  }
}

void scommand_set_redir_out(scommand self, char *filename) {
  assert(self != NULL);
  // destruye la redireccion in en caso de existir
  if (self->redir_out != NULL) {
    free(self->redir_out);
  }
  if (filename != NULL) {
    self->redir_out = filename;
    // free(filename);
    // filename = NULL;
  } else {
    self->redir_out = NULL;
  }
}

bool scommand_is_empty(const scommand self) {
  assert(self != NULL);
  bool scmd_is_empty = scommand_length(self) == 0;
  return scmd_is_empty;
}

unsigned int scommand_length(const scommand self) {
  assert(self != NULL);
  unsigned int length = g_queue_get_length(self->cmd_args);
  // assert((length == 0) == scommand_is_empty(self));
  return length;
}

char *scommand_front(const scommand self) {
  assert(self != NULL && !scommand_is_empty(self));
  char *cmd = g_queue_peek_head(self->cmd_args);
  assert(cmd != NULL);
  return cmd;
}

char *scommand_get_redir_in(const scommand self) {
  assert(self != NULL);
  char *filename = NULL;
  if (self->redir_in != NULL) {
    filename = self->redir_in;
  }
  return filename;
}

char *scommand_get_redir_out(const scommand self) {
  assert(self != NULL);
  char *filename = NULL;
  if (self->redir_out != NULL) {
    filename = self->redir_out;
  }
  return filename;
}

char *scommand_to_string(const scommand self) {
  assert(self != NULL);
  GString *glib_string = g_string_new(NULL);
  unsigned int length_cmd = g_queue_get_length(self->cmd_args);

  // Hace un "peek" en cada elemento de la queue el cual lo agrega al string
  for (unsigned int i = 0; i < length_cmd; i++) {
    glib_string =
        g_string_append(glib_string, g_queue_peek_nth(self->cmd_args, i));
    // Agrega un espacio salvo en el ultimo
    if (i != length_cmd - 1) {
      glib_string = g_string_append(glib_string, " ");
    }
  }
  // Agrega los redir luego del cmd y sus args
  if (self->redir_out != NULL) {
    g_string_append_printf(glib_string, " > %s", self->redir_out);
  }
  if (self->redir_in != NULL) {
    g_string_append_printf(glib_string, " < %s", self->redir_in);
  }

  // Pasa el string de la struct glib_string a un char * y lo libera
  char *cmd_string = glib_string->str;
  g_string_free(glib_string, FALSE);
  assert(scommand_is_empty(self) || scommand_get_redir_in(self) == NULL ||
         scommand_get_redir_out(self) == NULL || strlen(cmd_string) > 0);
  return cmd_string;
}

pipeline pipeline_new(void) {
  pipeline new_pipe = malloc(sizeof(struct pipeline_s));
  new_pipe->scmds = g_queue_new();
  new_pipe->wait = true;
  return new_pipe;
  assert(new_pipe != NULL && pipeline_is_empty(new_pipe) &&
         pipeline_get_wait(new_pipe));
}

pipeline pipeline_destroy(pipeline self) {
  assert(self != NULL);
  while (!g_queue_is_empty(self->scmds)) {
    scommand sc = g_queue_pop_head(self->scmds);
    scommand_destroy(sc);
  }
  g_queue_free(self->scmds);
  free(self);
  self = NULL;
  assert(self == NULL);
  return self;
}

void pipeline_push_back(pipeline self, scommand sc) {
  assert(self != NULL && sc != NULL);
  g_queue_push_tail(self->scmds, sc);
  assert(!pipeline_is_empty(self));
}

void pipeline_pop_front(pipeline self) {
  assert(self != NULL && !pipeline_is_empty(self));
  scommand sc = g_queue_pop_head(self->scmds);
  sc = scommand_destroy(sc);
}

void pipeline_set_wait(pipeline self, const bool w) {
  assert(self != NULL);
  self->wait = w;
}

bool pipeline_is_empty(const pipeline self) {
  assert(self != NULL);
  return pipeline_length(self) == 0;
}

unsigned int pipeline_length(const pipeline self) {
  assert(self != NULL);
  unsigned int length = g_queue_get_length(self->scmds);
  // assert((length == 0) == pipeline_is_empty(self));
  return length;
}

scommand pipeline_front(const pipeline self) {
  assert(self != NULL && !pipeline_is_empty(self));
  scommand front_scmd = g_queue_peek_head(self->scmds);
  assert(front_scmd != NULL);
  return front_scmd;
}

bool pipeline_get_wait(const pipeline self) {
  assert(self != NULL);
  bool wait = self->wait;
  return wait;
}

char *pipeline_to_string(const pipeline self) {
  assert(self != NULL);
  GString *glib_string = g_string_new(NULL);
  unsigned int length_pipeline = g_queue_get_length(self->scmds);

  // Hace un "peek" en cada elemento de la queue el cual lo agrega al string
  for (unsigned int i = 0; i < length_pipeline; i++) {
    scommand sc = g_queue_peek_nth(self->scmds, i);
    char *sc_str = scommand_to_string(sc);
    glib_string = g_string_append(glib_string, sc_str);
    free(sc_str);
    // Agrega un espacio salvo en el ultimo
    if (i < length_pipeline - 1) {
      glib_string = g_string_append(glib_string, " | ");
    } else if (pipeline_get_wait(self) == false) {
      glib_string = g_string_append(glib_string, " &");
    };
  }

  // Pasa el string de la struct glib_string a un char * y lo libera
  char *pipeline_string = glib_string->str;
  g_string_free(glib_string, FALSE);
  assert(pipeline_is_empty(self) || pipeline_get_wait(self) ||
         strlen(pipeline_string) > 0);
  return pipeline_string;
}
