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
#include <vector>
#include <string.h>
#include <unistd.h>

//return char ptr of so path
char* get_so_path(){
  //char result[152];
  //char result[152] = malloc(152);
  char* result = (char*)malloc(152);


  char exec_path[128];
  size_t end = readlink("/proc/self/exe",exec_path,128);
  //std::string str = "LD_PRELOAD=";

  exec_path[end-6] = 0;

  char ld_preload[14] =  "LD_PRELOAD=";
  char controlso[12] =  "control.so";
  //std::string myString(exec_path, 128);

  strcpy(result, ld_preload);
  strcat(result,exec_path);
  strcat(result,controlso);

  //char *cstr = new char[str.length() + 1];
  std::cout << result << std::endl;
  return result;
}

// Start child process
void start_child(std::string path, char *const argv[]) {
  // start process
  // ptrace(PTRACE_TRACEME, 0, 0, 0);
  // we need the shimmed child here

  //char exec_path[128];
  //readlink("/proc/self/exe",exec_path,128);
  //std::string str = "LD_PRELOAD=./control.so";
  //std::cout << exec_path << std::endl;



  //char *cstr = new char[str.length() + 1];
  //strcpy(cstr, str.c_str());
  
  // do stuff
  //delete [] cstr;


  //std::vector<char> control_path(str.c_str(), str.c_str() + str.size() + 1);
  //char* control_path = "LD_PRELOAD=./control.so".c_str();
  char* ld_preload = get_so_path();
  char* envp[] = {ld_preload, NULL};
  execve(path.c_str(), argv, envp);
  perror ("Error when executing child: ");

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

      
