#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <utility>

// long wordword;

typedef struct breakpoint {
  uintptr_t target_address;
  uintptr_t aligned_address;
  uintptr_t original_breakpoint_word;
} breakpoint;

// typedef int trap_inferior_t;
// typedef int trap_breakpoint_t;

void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
                           unsigned long data);
void breakpoint_remove(pid_t inferior, std::unique_ptr<breakpoint> bp);

std::unique_ptr<breakpoint> child_set_breakpoint(pid_t inferior,
                                                 char *location);
void dump_rip(pid_t child_pid);
