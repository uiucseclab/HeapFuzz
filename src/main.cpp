#include <iostream>
#include <memory>
#include <utility>

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
  auto child_pid = child_exec(conf.exec_name, child_args);
  //set breakpoint at snapshot addr
  std::unique_ptr<breakpoint> bp =
      child_set_breakpoint(child_pid, (char *)conf.snapshot_addr);
  //generate snapshot of readable/writeable pages
  generate_snapshot(child_pid);  // TODO: implement this

  //run child until termination
  child_finish(child_pid, std::move(bp));
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
