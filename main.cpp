#include <cstdlib>
#include "archlab.hpp"
#include <unistd.h>
#include"pin_tags.h"
#include"omp.h"
#include"papi.h"
#include<algorithm>
#include<cstdint>
#include"function_map.hpp"


uint array_size;

int main(int argc, char *argv[])
{
	
	std::vector<int> mhz_s;
	std::vector<int> default_mhz;
	load_frequencies();
	default_mhz.push_back(cpu_frequencies_array[1]);
	std::stringstream clocks;
	for(int i =0; cpu_frequencies_array[i] != 0; i++) {
		clocks << cpu_frequencies_array[i] << " ";
	}
	std::stringstream fastest;
	fastest << cpu_frequencies_array[0];

	archlab_add_multi_option<std::vector<int> >("MHz",
						    mhz_s,
						    default_mhz,
						    fastest.str(),
						    "Which clock rate to run.  Possibilities on this machine are: " + clocks.str());

	std::vector<std::string> functions;
	std::vector<std::string> default_functions;
	std::stringstream available_functions;
	for(auto & f: function_map::get()) {
		available_functions << "'" << f.first << "' ";
	}
	
	default_functions.push_back(function_map::get().begin()->first);
	archlab_add_multi_option<std::vector<std::string>>("function",
							   functions,
							   default_functions,
							   function_map::get().begin()->first,
							   "Which function to run.  Options are: " + available_functions.str() + "or 'ALL'.");

	std::vector<unsigned long int> sizes;
	std::vector<unsigned long int> default_sizes;
	default_sizes.push_back(1024*1024);
	archlab_add_multi_option<std::vector<unsigned long int> >("size",
					      sizes,
					      default_sizes,
					      "1024*1024",
					      "Size.  Pass multiple values to run with multiple sizes.");

	unsigned long int reps;
	archlab_add_option<unsigned long int >("reps",
					       reps,
					       1,
					       "1",
					       "How many times to repeat the experiment.");

	archlab_parse_cmd_line(&argc, argv);


	array_size  = *std::max_element(sizes.begin(), sizes.end());
	uint64_t * array = new uint64_t[array_size]; 


	theDataCollector->disable_prefetcher();

	if (std::find(functions.begin(), functions.end(), "ALL") != functions.end()) {
		functions.clear();
		for(auto & f : function_map::get()) {
			functions.push_back(f.first);
		}
	}
	
	for(auto & function : functions) {
		if (function_map::get().find(function) == function_map::get().end()) {
			std::cerr << "Unknown function: " << function <<"\n";
			exit(1);
		}
	}
	for(auto &mhz: mhz_s) {
		set_cpu_clock_frequency(mhz);
		for(auto & size: sizes ) {
			for(auto & function : functions) {
				std::cerr << "Running " << function << "\n";
				//pristine_machine();					
				theDataCollector->register_tag("size",size);
				uint64_t a = 1;
				for(uint64_t i = 0; i < array_size; i++) { // THis makes it deterministic, mildly interesting, and avoids page faults.
					array[i] = fast_rand(&a);
					//std::cerr << array[i] << "\n";
				}
				START_TRACE();
				{								
					ArchLabTimer timer;					
					flush_caches();
					//enable_prefetcher();
					//set_cpu_clock_frequency(cpu_frequencies[0]);
					NEW_TRACE(function.c_str());					
					timer.attr("function", function.c_str());				
					timer.attr("cmdlineMHz", mhz);
					timer.go();						
					DUMP_START_ALL(function.c_str(), false);
					auto func = function_map::get()[function];
					if (func.first == "1_array" || func.first == "sort")  {
						auto f = (uint64_t*(*)(uint64_t *, unsigned long int))(func.second);
						for(uint r = 0; r < reps; r++) {
							f(array,size);
						}
					} else if (func.first == "find_min") {
						auto f = (uint64_t(*)(uint64_t *, unsigned long int))(func.second);
						for(uint r = 0; r < reps; r++) {
							f(array,size);
						}
					} else if (func.first == "method") {
						auto f = (int(*)(register unsigned long int))(func.second);
						for(uint r = 0; r < reps; r++) {
							f(size);
						}
					} else {
						std::cerr << "unknown function type: " << func.first << "\n";
						exit(1);
					}
					// else if (func.first == "sum_impl") {
					// 	auto f = (uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))(func.second);
					// 	for(uint r = 0; r < reps; r++) {
					// 		f(array,size,);
					// 	}
					// }
					DUMP_STOP(function.c_str());
				}								
			}
		}
	}
	delete [] array;
	archlab_write_stats();
	return 0;
}
