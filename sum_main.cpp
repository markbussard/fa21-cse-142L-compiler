#include <cstdlib>
#include "archlab.hpp"
#include <unistd.h>
#include"pin_tags.h"
#include"omp.h"
#include"papi.h"
#include<algorithm>
#include<cstdint>
#include<unordered_set>
#include"function_map.hpp"
#include"fast_URBG.hpp"



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
	
	
	default_functions.push_back("ALL");
	archlab_add_multi_option<std::vector<std::string>>("function",
							   functions,
							   default_functions,
							   "sum_of_locations",
							   "Which function to run.  Options are: " + available_functions.str() + "or 'ALL'.");
	

	std::vector<uint64_t> space_sizes;
	std::vector<uint64_t> default_space_sizes;
	default_space_sizes.push_back(16*128*1024);
	archlab_add_multi_option<std::vector<uint64_t> >("space-size",
							 space_sizes,
							 default_space_sizes,
							 "2097152",
							 "Search space size.  Pass multiple values to run with multiple space_sizes.");
	std::vector<uint64_t> query_counts;
	std::vector<uint64_t> default_query_counts;
	default_query_counts.push_back(1024);
	archlab_add_multi_option<std::vector<uint64_t> >("queries",
							 query_counts,
							 default_query_counts,
							 "1024",
							 "Number of queries to run.  Pass multiple values to run with multiple query_counts.");

	uint64_t reps;
	archlab_add_option<uint64_t>("reps",
				     reps,
				     1,
				     "1",
				     "How many times to repeat the experiment.");

	uint64_t iters;
	archlab_add_option<uint64_t>("iters",
				     iters,
				     1,
				     "1",
				     "How many times to run the function in each experiment.");
	uint64_t seed;
	archlab_add_option<uint64_t>("seed",
				     seed,
				     1,
				     "1",
				     "Seed to use for the random number generator.");

	archlab_parse_cmd_line(&argc, argv);


	uint64_t max_space_size = *std::max_element(space_sizes.begin(), space_sizes.end());
	uint64_t * search_space = new uint64_t[max_space_size];
	for(uint64_t i = 0; i < max_space_size; i++) {
		search_space[i] = i;
	}
	std::shuffle(search_space, &search_space[max_space_size], fast_URBG(seed));

	// for(uint64_t a = 0; a < max_space_size; a++) {
	// 	std::cerr << search_space[a] << "\n";
	// }
	uint64_t max_query_count = *std::max_element(query_counts.begin(), query_counts.end());
	uint64_t * query_list = new uint64_t[max_query_count];
	uint64_t _seed = seed;
	for(uint i = 0; i < max_query_count; i++) {
		query_list[i] = fast_rand(&_seed) % (max_space_size * 2);
	}

	// std::cerr << "\n";
	// for(uint64_t a = 0; a < max_query_count; a++) {
	// 	std::cerr << query_list[a] << "\n";
	// }
	
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
		for(auto & space_size: space_sizes ) {
			for(auto & query_count: query_counts ) {
				for(uint r = 0; r < reps; r++) {
					
					for(auto & function : functions) {
						uint64_t a = 0;
						//pristine_machine();					
						theDataCollector->register_tag("space_size",space_size);
						theDataCollector->register_tag("query_count",query_count);
						theDataCollector->register_tag("seed",seed);
						theDataCollector->register_tag("result", 0);
						uint64_t answer;
						START_TRACE();
						{								
							ArchLabTimer timer;					
							flush_caches();
							//enable_prefetcher();
							//set_cpu_clock_frequency(cpu_frequencies[0]);
							NEW_TRACE(function.c_str());					
							timer.attr("function", function.c_str());				
							timer.attr("cmdlineMHz", mhz);
							timer.attr("rep", r);
							timer.go();						
							DUMP_START_ALL(function.c_str(), false);
//						uint * t =
							
							auto f = function_map::get()[function];
							if (f.first == "sum_impl") {
								auto func = (uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))(f.second);
								for(uint r = 0; r < reps; r++) {
									answer = func(search_space, space_size, query_list, query_count);
								}
							}
							
							DUMP_STOP(function.c_str());					
							//if (t)
							//delete t;						
						}
						theDataCollector->set_tag("result", answer);

						std::cerr << a << "\n";
					}
				}
			}
		}

	}
	delete [] search_space;
	delete [] query_list;
	archlab_write_stats();
	return 0;
}
