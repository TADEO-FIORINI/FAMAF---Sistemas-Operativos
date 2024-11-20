#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char **argv)
{
  int pid = getpid();
  pstat(pid);
  // while (1) {
  //   if(uptime()>=50){ //me da tiempo a correr el programa
  //       int j = 0;
  //       while(uptime() < 51) {
  //         j++;
  //         // i += uptime();
  //       }
  //       printf("veces que corrio el while: %d\n", j);
  //        
  //     break;
  //   }
  //  }
  return 0;
}

