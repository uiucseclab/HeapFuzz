#include <iostream>
#include <string>
#include <vector>

typedef struct page_link {
  unsigned long start;
  unsigned long end;
  bool readable;
  bool writeable;
  bool executable;
  unsigned long offset;
  std::string pathname;

} page_link;

// this may be broken up over several structs in the
// future, but for now its just a big buket of state
typedef struct state {
  std::vector<page_link> page_map;
} state;

void generate_snapshot(pid_t child);