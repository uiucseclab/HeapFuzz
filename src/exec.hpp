#ifndef EXEC_H
#define EXEC_H

#include <memory>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include "instrument.hpp"

//load child, return just before first instruction
pid_t child_exec(const std::string path, char *const argv[]);
//run child until termination
void child_finish(pid_t child, std::unique_ptr<breakpoint> bp);

#endif //EXEC_H