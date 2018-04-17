#include <iostream>
#include <memory>
#include <utility>
#include <unistd.h>

#include "instrument.hpp"
#include "snapshot.hpp"
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
  //load child process, returns just before first instruction with child stopped
  prepare_fork_server();
  //dont use the return value here because we actually want the grandchild
  child_exec(conf.exec_name, child_args);

  int status, pid;
  auto msg = "hi";
  read(fuzzer_pipe[0], &status, 4); // get ready signal from child
  //Now we can write to the server to fork
  write(server_pipe[1], msg,  4);
  //After it forks we read its PID
  read(fuzzer_pipe[0], &pid,  4);
  //PTRACE_ATTACH to the grandchild
  //read the wait status from the child
  read(fuzzer_pipe[0], &status, 4);
  //run child until termination

// this was a snapshot thing
//  child_finish(child_pid, std::move(bp));
}


//Using main only to parse commandlines and create config struct
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
