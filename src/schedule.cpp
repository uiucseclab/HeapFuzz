#include <string>
#include <vector>
#include <iostream>
#include <random>
#include <queue>
#include <map>

std::mt19937 seed(12345);
std::queue<std::string> scheduler;

//Maps call address of mallocs to number it has been called
std::map<address,unsigned int> malloc_map;
std::map<address,unsigned int> free_map;


enum mem_op_type {Malloc,Free};
typedef unsigned long address;

typedef struct mem_op {
  mem_op_type call_type;
  address called_from;
  unsigned long parameter;
} mem_op;

typedef std::vector<mem_op> trace;



std::string mutate(std::string input){

	 //seeded for reproducibility
	std::uniform_int_distribution<int> char_choice(1, input.length());
	std::uniform_int_distribution<int> len_change_choice(1, 2);
	std::uniform_int_distribution<int> rand_char(0, 255);


	auto num_mutations = 3;
	for (auto i = 0; i < num_mutations; i++)
	{
		auto char_to_mutate = char_choice(seed);
		input[char_to_mutate] = rand_char(seed);
	
		auto lcc = len_change_choice(seed);
		if(lcc == 1){
			input += (rand_char(seed) + rand_char(seed) + rand_char(seed));
		}
		else {
			if(input.length() > 3){
				input.pop_back();
				input.pop_back();
				input.pop_back();

			}
		}
	}
	return input;
	
}


//mutate string n times and add them to queue
void schedule(int num_to_schedule,std::string input){

	for (auto i = 0; i < num_to_schedule; i++)
	{
		scheduler.push(mutate(input));
	}
}

//boolean value to determine whether to enque
//don't you dare change the name
//Todo: make this good
//currently just schedules 3 whenever we see a new malloc or free call
int rateTrace(trace myTrace){
	auto rating = 0;

	for(auto &call : myTrace){
		auto call_type  call.call_type;
  		auto called_from = call.called_from;
  		//unsigned long parameter;
  		if(call_type == Malloc){
  			if(my_map.find( key ) == my_map.end()){
  				malloc_map[called_from] = 1;
  				rating += 3;
  			}
  			else{
				malloc_map[called_from]++;
  			}
  		}

  		if(call_type == Free){
  			if(my_map.find( key ) == my_map.end()){
  				free_map[called_from] = 1;
  				rating += 3;
  			}
  			else{
				free_map[called_from]++;
  			}
  		}


	}
	return 1;
}


int main(){
	std::string testStr = "testtesttest";
	std::string mutated = mutate(testStr);
	std::cout << testStr << '\n' << mutated << std::endl;


	//for(auto i = 0; i < 100; i++){
	//	mutated = mutate(mutated);
	//	std::cout << mutated <<  std::endl;
	//}

	mem_op m1 = {Malloc,0x1000,32};
	mem_op m2 = {Malloc,0x1000,32};
	mem_op m3 = {Malloc,0x1000,32};
	trace dummy_trace = {m1,m2,m3};

	scheduler.push(testStr);

	//will eventually be a while !scheduler.empty()
	for (auto i = 0; i < 10; i++)
	{
		//str = scheduler.pop()
		//trace = program.run(str)
		auto val = rateTrace(dummy_trace);
		//auto val = rateTrace(trace);
		//schedule(val,str);
	}


}


