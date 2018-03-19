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
#include "instrument.hpp"
#include "snapshot.hpp"

// TODO: rename/refactor, documentation, write tests
// TODO: more generic makefile
// TODO: better error checking

int const ignored_arg = 0;
static const void *no_continue_signal = 0;

// hold commandline stuff
typedef struct config {
  std::string exec_name;
  unsigned long snapshot_addr;
} config;

// Input:
// Output:
void start_child(std::string path, char *const argv[]) {
  // start process
  ptrace(PTRACE_TRACEME, 0, 0, 0);
  execv(path.c_str(), argv);
}

//
void child_continue(pid_t child, std::unique_ptr<breakpoint> bp) {
  auto pid = child;

  ptrace(PTRACE_CONT, pid, ignored_arg, no_continue_signal);
  while (1) {
    int status;
    waitpid(pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      struct user_regs_struct regs;

      generate_snapshot(child);  // TODO: implement this
      breakpoint_remove(child, std::move(bp));

      ptrace(PTRACE_GETREGS, pid, ignored_arg, &regs);
      regs.rip -= 1;
      ptrace(PTRACE_SETREGS, pid, ignored_arg, &regs);

      ptrace(PTRACE_CONT, pid, ignored_arg, no_continue_signal);
    } else if (WIFEXITED(status)) {  // process stopped regularly
      return;
    } else {
      fprintf(stderr, "Child Unexpectedly stopped: 0x%x\n", status);
      abort();
    }
  }
}

// Input: pid of child
// Output:
//
static void attach_to_child(pid_t pid) {
  int status;
  waitpid(pid, &status, 0);

  if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
    return;
  } else {
    fprintf(stderr, "Unexpected status for child %d when attaching\n", pid);
    abort();
  }
}

// Input: Path to child, args to pass to child
// Output: pid of child
pid_t child_exec(const std::string path, char *const argv[]) {
  pid_t result;

  do {
    result = fork();
    switch (result) {
      case 0:  // child
        start_child(path, argv);
        break;
      case -1:  // error
        break;
      default:  // parent
        attach_to_child(result);
        break;
    }
  } while (result == -1 && errno == EAGAIN);

  return result;
}

void run(config conf) {
  char *child_args[1] = {0};  // arguments to child process
  auto child_pid = child_exec(conf.exec_name, child_args);

  std::unique_ptr<breakpoint> bp =
      child_set_breakpoint(child_pid, (char *)conf.snapshot_addr);
  dump_rip(child_pid);
  child_continue(child_pid, std::move(bp));
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <Exec Path> <Snapshot Address>"
              << std::endl;
    return 1;
  }

  std::string name = argv[1];  // name of executable to fuzz
  unsigned long main_address =
      std::stoul(argv[2], nullptr, 16);  // address to reset fuzzing to
  config conf = {name, main_address};

  run(conf);
}
