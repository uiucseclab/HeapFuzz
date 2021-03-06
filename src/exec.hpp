#ifndef EXEC_H
#define EXEC_H

#include <memory>
#include <utility>
#include <stdio.h>
#include <stdlib.h>
//#include "instrument.hpp"

//load child, return just before first instruction
pid_t child_exec(const std::string path, char *const argv[], int* pipes);

//sets up pipes befor the fork
void prepare_comm_pipes(int *);
void prepare_fork_server(int *, int *);
void close_parent_pipes(int , int );

#endif //EXEC_H
