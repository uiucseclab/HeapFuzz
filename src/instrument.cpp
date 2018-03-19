#include "instrument.hpp"

std::unique_ptr<breakpoint> breakpoint_set(pid_t inferior,
                                           std::unique_ptr<breakpoint> bp);

// wrapper around ptrace for easier writes to memory
void ptrace_util_poke_text(pid_t pid, unsigned long target_address,
                           unsigned long data) {
  int result =
      ptrace(PTRACE_POKETEXT, pid, (void *)target_address, (void *)data);
  if (result != 0) {
    perror("PTRACE_POKETEXT: ");
    abort();
  }
}

void breakpoint_remove(pid_t child, std::unique_ptr<breakpoint> bp) {
  pid_t child_pid = child;

  ptrace_util_poke_text(child_pid, bp->target_address,
                        bp->original_breakpoint_word);
}

void dump_rip(pid_t child_pid) {
  user_regs_struct regs;
  ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
  printf("0x%llx\n", regs.rip);
}

std::unique_ptr<breakpoint> breakpoint_set(pid_t child,
                                           std::unique_ptr<breakpoint> bp) {
  const uintptr_t int3_opcode = 0xCC;
  auto pid = child;
  auto target_offset = bp->target_address - bp->aligned_address;

  uintptr_t modified_word;
  modified_word = bp->original_breakpoint_word;
  modified_word &= ~(0xFFUL << (target_offset * 8));
  modified_word |= int3_opcode << (target_offset * 8);
  ptrace_util_poke_text(pid, bp->aligned_address, modified_word);
  return bp;
}

std::unique_ptr<breakpoint> child_set_breakpoint(pid_t child, char *location) {
  const uintptr_t target_address = (uintptr_t)location;
  auto aligned_address = target_address & ~(0x7UL);

  auto original_breakpoint_word =
      ptrace(PTRACE_PEEKTEXT, child, (void *)target_address, 0);
  std::unique_ptr<breakpoint> bp = std::make_unique<breakpoint>();

  bp->target_address = target_address;
  bp->aligned_address = aligned_address;
  bp->original_breakpoint_word = original_breakpoint_word;

  bp = breakpoint_set(child, std::move(bp));

  return bp;
}