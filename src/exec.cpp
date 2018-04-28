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
  // ptrace(PTRACE_TRACEME, 0, 0, 0);
  // we need the shimmed child here
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
void close_parent_pipes(int server_read, int fuzzer_write) {
  close(server_read);
  close(fuzzer_write);

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
        break;
    }
  } while (result == -1 && errno == EAGAIN);

  return result;
}

void prepare_fork_server(int *server_pipe, int *fuzzer_pipe){
  pipe(server_pipe);
  pipe(fuzzer_pipe);

  //TODO: do we need to set FD_CLOEXEC on the other ends to not have to close them 
  // in the server?
  auto read_end = fcntl(server_pipe[0], F_DUPFD, 198); //fork server reading end at 198
  auto write_end = fcntl(fuzzer_pipe[1], F_DUPFD, 199); //fork server writing end at 199
  
  //Close the old fds and replaced them with the duped fds
  close(server_pipe[0]);
  close(fuzzer_pipe[1]);
  server_pipe[0] = read_end;
  fuzzer_pipe[1] = write_end;

  std::cout << server_pipe[0] << " " << server_pipe[1] << std::endl;
  std::cout << fuzzer_pipe[0] << " " << fuzzer_pipe[1] << std::endl; 
}

      
