#include <stdlib.h>
#include <stdio.h>
#include "../../src/shim.h"

int main()
{
  start_mtrace_fork_server();
  puts("Hello World!");

  void *a = calloc(1, 4);
  a = realloc(a, 256);
  free(a);

  return 0;
}
