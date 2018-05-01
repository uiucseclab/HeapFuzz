#include "schedule.hpp"
#include "exec.hpp"

std::mt19937 seed(12345);
std::map<address,unsigned int> malloc_map;
std::queue<std::string> scheduler;


//Maps call address of mallocs to number it has been called
//std::map<address,unsigned int> free_map;


std::string mutate(std::string input){

	 //seeded for reproducibility
	std::uniform_int_distribution<int> char_choice(1, input.length());
	std::uniform_int_distribution<int> len_change_choice(1, 2);
	std::uniform_int_distribution<int> rand_char(0, 255);

	//for(int i =0; i<10; i++)
	//	std::cout << "rand val is: " << rand_char(seed) << std::endl;
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
void schedule(int num_to_schedule,std::vector<std::string> input){

	for(auto i =0; i < (int)input.size(); i++)
		scheduler.push(input[i]);
	for (auto i = 0; i < num_to_schedule; i++)
	{
		scheduler.push(mutate(input[i]));
	}
}

//boolean value to determine whether to enque
//don't you dare change the name
//Todo: make this good
//currently just schedules 3 whenever we see a new malloc or free call
int rateTrace(trace myTrace){
	auto rating = 0;

	for(auto &call : myTrace){
		auto call_type = call.call_type;
  		auto called_from = call.called_from;
  		auto parameter = call.parameter;
  		//auto parameter2 = call.parameter2;

  		//unsigned long parameter;
  		if(call_type == (Malloc || Calloc)){
  			if(malloc_map.find( called_from ) == malloc_map.end()){
  				malloc_map[called_from] = 1;
  			}
  			else{
				malloc_map[called_from]++;
				rating += 3;

  			}
  		}
  		else if(call_type == Free){
  			if(malloc_map.find( called_from ) == malloc_map.end()){
  				malloc_map[called_from] = -1; //flag?
  				rating += 3;
  			}
  			else{
  				if(malloc_map[called_from] == 0)
  					rating += 3;
				malloc_map[called_from]--;
  			}
  		}
  		else if(call_type == Realloc){
  			if(malloc_map.find(parameter) == malloc_map.end())
  			{
  				if(!parameter){
  				    malloc_map[parameter] = 1;
  				}
  				else{
  					malloc_map[parameter] = -1;
  					rating += 3;
  				}
  			}
  			else{
  				malloc_map[called_from] = 1;
  				malloc_map[parameter]--;
  			}
  		}
  	}
	
	return rating;
}

/*int init_schedule(std::string input, trace myTrace){
	std::string testStr = input;
	std::string mutated = mutate(testStr);
	std::cout << testStr << '\n' << mutated << std::endl;


	//for(auto i = 0; i < 100; i++){
	//	mutated = mutate(mutated);
	//	std::cout << mutated <<  std::endl;
	//}

	mem_op m1 = {Malloc,0x1000,32, 0};
	mem_op m2 = {Malloc,0x1000,32, 0};
	mem_op m3 = {Malloc,0x1000,32, 0};
	mem_op m4 = {Free,0x1000,32, 0};

	trace dummy_trace = myTrace;

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

	while(!scheduler.empty())
	{
		std::string lol = scheduler.front();
		scheduler.pop();
		auto val = rateTrace(dummy_trace);
		if(val > 0){
			schedule(val, lol);
		}
	}

	for(auto &iter : my_map)
	{
		if(iter.second != 0)
		{
			std::cout << "Error detected \n"; 
			return 1;
		}
	}

	return 0;
}
*/

