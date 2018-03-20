#include "exec.hpp"
#include <sys/ptrace.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/user.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>
#include "snapshot.hpp"

int const ignored_arg = 0;
static const void *no_continue_signal = 0;



// Start child process
void start_child(std::string path, char *const argv[]) {
  // start process
  ptrace(PTRACE_TRACEME, 0, 0, 0);
  execv(path.c_str(), argv);
}

//allow th child to continue until it sends another signal
//if that signal is sigtrao, remove the breakpoint and generate
//a snapshot, else fail
//TODO: on other signals, instead of quit, log them and reset
void child_finish(pid_t child, std::unique_ptr<breakpoint> bp) {
  auto pid = child;

  ptrace(PTRACE_CONT, pid, ignored_arg, no_continue_signal);
  while (1) {
    int status;
    waitpid(pid, &status, 0);

    if (WIFSTOPPED(status) && WSTOPSIG(status) == SIGTRAP) {
      struct user_regs_struct regs;

      breakpoint_remove(child, std::move(bp));

      ptrace(PTRACE_GETREGS, pid, ignored_arg, &regs);
      regs.rip -= 1;
      ptrace(PTRACE_SETREGS, pid, ignored_arg, &regs);

      ptrace(PTRACE_CONT, pid, ignored_arg, no_continue_signal);
    } else if (WIFEXITED(status)) {  // process stopped regularly
      return;
    } else {
      //std::cout << strsignal()
      fprintf(stderr, "Child Unexpectedly stopped: 0x%x\n", status);
      abort();
    }
  }
}

// Input: pid of child
// Output:
//wait for child, to pass control back to parent process
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
//spawns child in a new process and attaches to it

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