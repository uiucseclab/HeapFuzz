#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include <iostream>
#include <string>
#include <vector>

// this may be broken up over several structs in the
// future, but for now its just a big buket of state
/*
typedef struct state {
  std::vector<page_link> page_map;
} state;
*/
void generate_snapshot(pid_t child);

#endif //SNAPSHOT_H
