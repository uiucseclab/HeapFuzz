#ifndef SCHEDULE_H
#define SCHEDULE_H


#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <queue>
#include <map>

enum mem_op_type {Malloc,Free, Calloc, Realloc};
typedef unsigned long address;

typedef struct mem_op {
  mem_op_type call_type;
  address called_from;
  unsigned long parameter;
  unsigned long parameter2;
} mem_op;

typedef std::vector<mem_op> trace;

//int init_schedule(int seed, trace myTrace);
std::string get_next();
std::string mutate(std::string input);
void schedule(int num_to_schedule, std::vector<std::string> input);
int rateTrace(trace myTrace);


#endif //SCHEDULE_H
