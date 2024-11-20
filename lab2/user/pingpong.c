#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if(argc < 2){
    fprintf(2, "ERROR no hay parametros\n");
    exit(1);
  } 
  unsigned int n = atoi(argv[1]);
  unsigned int r = 0;
  unsigned int t = 1;
  int pid;
  
  if (n < 1) { 
    fprintf(2, "ERROR el numero de round tiene que ser mayor a 1\n");
    exit(1);
  }

  while(sem_open(r, 1) == 0)
    r++;
  while(sem_open(t, 0) == 0)
    t++;

  if ((pid = fork()) == -1) {
    fprintf(2, "ERROR en el fork");
    exit(1);
  }
  if (pid == 0) {
    while(n > 0){
      sem_down(t);
      n--;
      printf("    Pong\n");
      sem_up(r);
    }
  } else {
    while(n > 0) {
      sem_down(r);
      n--;
      printf("Ping\n");
      sem_up(t);
    }
    wait(0);
    sem_close(r);
    sem_close(t);
  }
  exit(0);
}

