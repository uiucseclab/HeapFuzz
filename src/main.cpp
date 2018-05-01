#include <iostream>
#include <memory>
#include <utility>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <vector>

#include "exec.hpp"

// TODO: write tests
// TODO: more generic makefile
// TODO: better error checking

// holds commandline args
typedef struct config {
  std::string exec_name;
  std::vector<char*> args;
  char input_method;
} config;

void print_wait_status(int status){
  if (WIFEXITED(status)) {
    std::cout << "Child exited with "<< WEXITSTATUS(status) << std::endl;
  }
  if (WIFSIGNALED(status)) {
    std::cout << "Child exited via signal "<< WTERMSIG(status) << std::endl;
  }
}

//base point for all execution
void run(config conf) {
  char **child_args = &conf.args[0];
  int fuzzer_pipe[2], server_pipe[2], trace_pipe[2], stdin_pipe[2];

  //We should probably ALWAYS set up these pipes
  //We need the trace pipe and keeping the stdin pipe simplifies things
  prepare_comm_pipes(trace_pipe);
  pipe(stdin_pipe);

  //So now we can either use the fork server and pass stdin
  //or exec the child with different args
  if(conf.input_method == 's'){ 
    prepare_fork_server(server_pipe, fuzzer_pipe);
    child_exec(conf.exec_name, child_args, stdin_pipe);

    uint32_t status, pid;
    int32_t msg = 0xcafebabe;
    read(fuzzer_pipe[0], &status, 4); // get ready signal from child
    assert(status == 0xdeadbeef);

    for(int i = 0; i < 10; i++){
      //Now we can write to the server to fork
      write(server_pipe[1], &msg,  4);
      //After it forks we read its PID
      read(fuzzer_pipe[0], &pid,  4);
      //std::cout << "Grandchild PID: "<< std::dec << pid << std::endl;
      //run child until termination
      //read the wait status from the child
      read(fuzzer_pipe[0], &status, 4);
      print_wait_status(status);
    }
  } else if(conf.input_method == 'a') {
    //generate the arg to launch the child with
    //launch the child and wait on it
    int status;
    auto pid = child_exec(conf.exec_name, child_args, stdin_pipe);
    close(stdin_pipe[1]);
    //get its trace
    //read/epoll
    waitpid(pid, &status, WUNTRACED);
    print_wait_status(status);
    //repeat
  }
}

//Using main only to parse commandlines and create config struct
int main(int argc, char *argv[]) {
  if (argc < 3) {
    std::cerr << "Usage: " << argv[0] << " <Type> <Exec Path> [Args]" << std::endl
              << "Where Type is :\t a \t Fuzz arguments" << std::endl
              << "               \t s \t Fuzz stdin"  << std::endl
              << "               \t f \t Fuzz file input"
              << std::endl;
    return 1;
  }
  char fuzz_type = argv[1][0];
  std::string name = argv[2];  // name of executable to fuzz
  if(argc > 3){
    name = argv[2];
  }

  config conf = {name, std::vector<char*>(argv+2, argv+argc), fuzz_type};

  run(conf);
}
