#include <stdio.h>
#include "../../src/shim.h"

int main()
{
  start_mtrace_fork_server();
  puts("Hello World!");

  return 0;
}
