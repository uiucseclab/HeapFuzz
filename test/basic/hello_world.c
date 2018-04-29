#include <stdio.h>
<<<<<<< Updated upstream
#include "../../src/shim.h"
=======
#include <dlfcn.h>
>>>>>>> Stashed changes

int main()
{
  start_mtrace_fork_server();
  puts("Hello World!");

  return 0;
}
