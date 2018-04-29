#include <iostream>
#include <memory>
#include <utility>
#include <unistd.h>
#include <cassert>

#include "exec.hpp"

// TODO: write tests
// TODO: more generic makefile
// TODO: better error checking

// holds commandline args
typedef struct config {
  std::string exec_name;
  unsigned long snapshot_addr;
} config;



//base point for all execution
void run(config conf) {
  char *child_args[1] = {0};
  int fuzzer_pipe[2], server_pipe[2], trace_pipe[2];

  //load child process, returns just before first instruction with child stopped
  prepare_fork_server(server_pipe, fuzzer_pipe, trace_pipe);
  //std::cout << "Fork server ready" << std::endl;
  //dont use the return value here because we actually want the grandchild
  child_exec(conf.exec_name, child_args);
  //close_parent_pipes(server_pipe[0], fuzzer_pipe[1]);
  //std::cout << "Child forked" << std::endl;
  uint32_t status, pid;
  int32_t msg = 0xcafebabe;
  read(fuzzer_pipe[0], &status, 4); // get ready signal from child
  assert(status == 0xdeadbeef);

  for(int i = 0; i < 10; i++){
    //std::cout << std::hex << status << std::endl;
    //Now we can write to the server to fork
    write(server_pipe[1], &msg,  4);
    //After it forks we read its PID
    read(fuzzer_pipe[0], &pid,  4);
    std::cout << "Grandchild PID: "<< std::dec << pid << std::endl;
    //PTRACE_ATTACH to the grandchild
    //read the wait status from the child
    read(fuzzer_pipe[0], &status, 4);
    //run child until termination
    if (WIFEXITED(status)) {
        std::cout << "Child exited with "<< WEXITSTATUS(status) << std::endl;
    }
    if (WIFSIGNALED(status)) {
        std::cout << "Child exited via signal "<< WTERMSIG(status) << std::endl;
    }
  }
}


//Using main only to parse commandlines and create config struct
int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <Exec Path>"
              << std::endl;
    return 1;
  }

  std::string name = argv[1];  // name of executable to fuzz
  config conf = {name, 0};

  run(conf);
}
