#ifndef INSTRUMENT_H
#define INSTRUMENT_H


#include <memory>
#include <utility>


typedef struct breakpoint {
  uintptr_t target_address;
  uintptr_t aligned_address;
  uintptr_t original_breakpoint_word;
} breakpoint;


void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
                           unsigned long data);
void breakpoint_remove(pid_t inferior, std::unique_ptr<breakpoint> bp);

std::unique_ptr<breakpoint> child_set_breakpoint(pid_t inferior,
                                                 char *location);
void dump_rip(pid_t child_pid);

#endif //INSTRUMENT_H
