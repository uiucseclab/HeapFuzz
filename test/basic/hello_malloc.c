#include <stdlib.h>
#include <stdio.h>
#include "../../src/shim.h"

int main()
{
  start_mtrace_fork_server();
  puts("Hello World!");

  void *a = malloc(4);
  free(a);

  return 0;
}
