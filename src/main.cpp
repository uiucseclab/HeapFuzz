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
      //analyze the trace and queue "interesting" strings
      trace myTrace;
      char callByte;
      do{
        uint64_t size;
        uint64_t addr;
        read(trace_pipe[0], &callByte, 1);
        if(callByte == 'm')
        {
      	  read(trace_pipe[0], &size, sizeof(size_t));
      	  read(trace_pipe[0], &addr, sizeof(size_t));
      	  mem_op v ={Malloc, (address) addr, (unsigned long) size, 0};
      	  myTrace.push_back(v);
        }
        if(callByte == 'f')
        {
      	  read(trace_pipe[0], &addr, sizeof(size_t));
      	  mem_op v ={Free, (address) addr, 0, 0};
      	  myTrace.push_back(v);
        }
        //read the wait status from the child
        read(fuzzer_pipe[0], &status, 4);
      } while((WIFEXITED(status) && WIFSIGNALED(status)) == 0);

      // TODO: change args to have input folder
      init_folder(conf.args[0]);
      std::vector<std::string> init_paths = get_input_paths(conf.args[0]);
      std::vector<std::string> init_seeds = get_init_seeds(init_paths, conf.args[0]);
      std::vector<std::string> init_input = init_seeds;
      schedule(rateTrace(myTrace), init_input);
      //run child until termination
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
