#include <iostream>
#include <memory>
#include <utility>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <vector>
#include <dirent.h>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include "exec.hpp"
#include "schedule.hpp"

// TODO: write tests
// TODO: more generic makefile
// TODO: better error checking

// holds commandline args
typedef struct config {
  std::string exec_name;
  std::vector<char*> args;
  char input_method;
} config;


std::string file_to_seed(std::string path){
  std::ifstream t(path);
  std::stringstream buffer;
  buffer << t.rdbuf();
  return buffer.str();
}

void init_folder(char* path){
      mkdir(path, 0777); 
}

std::vector<std::string> get_init_seeds(std::vector<std::string> paths,std::string input_folder){ 
  std::vector<std::string> init_seeds;
  for (std::vector<std::string>::iterator i = paths.begin(); i != paths.end(); ++i)
  {
    init_seeds.push_back(input_folder + *i);
  }
  return init_seeds;
}

std::vector<std::string> get_input_paths(char* input_path){
  std::vector<std::string> init_seeds;

  DIR *dir;
    struct dirent *ent;
    if ((dir = opendir (input_path)) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    //printf ("%s\n", ent->d_name);
    if(strcmp(ent->d_name,"..") && strcmp(ent->d_name,".")){
    
      //init_seeds.push_back(file_to_seed(strcat("in/",ent->d_name)));
      init_seeds.push_back(ent->d_name);
    }
    }
    closedir (dir);
  } else {
    /* could not open directory */
    perror ("");
    return init_seeds;
  }
  return init_seeds;
}


void print_wait_status(int status){
  if (WIFEXITED(status)) {
    std::cout << "Child exited with "<< WEXITSTATUS(status) << std::endl;
  }
  if (WIFSIGNALED(status)) {
    std::cout << "Child exited via signal "<< WTERMSIG(status) << std::endl;
  }
}

trace read_trace(int trace_pipe){
  trace myTrace;
  char callByte;
  while(1){
    uint64_t size;
    uint64_t addr;
    read(trace_pipe, &callByte, 1);
    if(errno == EAGAIN || errno == EWOULDBLOCK){
      std::cout << "Done reading" << std::endl;
      break;
    }
    if(callByte == 'm')
    {
      read(trace_pipe, &size, sizeof(size_t));
      read(trace_pipe, &addr, sizeof(size_t));
      mem_op v ={Malloc, (address) addr, (unsigned long) size, 0};
      myTrace.push_back(v);
    }
    if(callByte == 'f')
    {
      read(trace_pipe, &addr, sizeof(size_t));
      mem_op v ={Free, (address) addr, 0, 0};
      myTrace.push_back(v);
    }
  }
  
  return myTrace;
}

//base point for all execution
void run(config conf) {
  char **child_args = &conf.args[0];
  int fuzzer_pipe[2], server_pipe[2], trace_pipe[2], stdin_pipe[2];
  int status;

  //We should probably ALWAYS set up these pipes
  //We need the trace pipe and keeping the stdin pipe simplifies things
  prepare_comm_pipes(trace_pipe);
  std::vector<std::string> init_input (1, "testtesttest"); //= init_seeds;
  schedule(1, init_input); //Put a seed into the scheduler

  //So now we can either use the fork server and pass stdin
  //or exec the child with different args
  if(conf.input_method == 's'){ 
    prepare_fork_server(server_pipe, fuzzer_pipe);

    for(int i = 0; i < 10; i++){
      pipe(stdin_pipe);
      auto pid = child_exec(conf.exec_name, child_args, stdin_pipe);
      auto in = get_next();
      write(stdin_pipe[1], in.c_str(), in.size()+1);
      close(stdin_pipe[1]); //We have to close this so the target doesn't hang, but we can't reopen it either...

      waitpid(pid, &status, WUNTRACED);
      print_wait_status(status);
      trace myTrace = read_trace(trace_pipe[0]);
      schedule(rateTrace(myTrace), init_input);
    }
  } else if(conf.input_method == 'a') {
    //generate the arg to launch the child with
    //launch the child and wait on it
    for(int i = 0; i < 10;  i++){
      char* terminator = {0};
      auto in = get_next();
      auto copy = (char*)malloc(in.size()+1);
      strcpy(copy, in.c_str());

      conf.args.pop_back(); // remove the terminator
      conf.args.push_back(copy);
      conf.args.push_back(terminator);
      auto pid = child_exec(conf.exec_name, &conf.args[0], stdin_pipe);
      close(stdin_pipe[1]);
      conf.args.erase(conf.args.end()-2);

      //get its trace
      //read/epoll
      waitpid(pid, &status, WUNTRACED);
      print_wait_status(status);
      trace myTrace = read_trace(trace_pipe[0]);
      schedule(rateTrace(myTrace), init_input);
    }
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
  char* terminator = {0};
  std::vector<char*> args = std::vector<char*>(argv+2, argv+argc);
  args.push_back(terminator);
  config conf = {name, args, fuzz_type};

  run(conf);
}
