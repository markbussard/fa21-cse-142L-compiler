#include "gtest/gtest.h"
#include"function_map.hpp"
#include<algorithm>
#include "archlab.hpp"
#include"fast_URBG.hpp"
#include <random>
 
namespace Tests {

      	
	class CorrectnessTests :  public ::testing::Test {
		
	};

	TEST_F(CorrectnessTests, sum_of_locations) {

		std::random_device rd;
		std::uniform_int_distribution<int> dist(1024, 1024*1024);
		//auto func = (uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))(uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))(f.second);
		auto sum_of_locations = (uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))function_map::get()["sum_of_locations"].second;
		auto sum_of_locations_solution = (uint64_t(*)(uint64_t *, unsigned long int, uint64_t *, unsigned long int))function_map::get()["sum_of_locations_solution"].second;
		uint64_t seed = dist(rd);
		uint64_t search_space_size = (1 << 20);
		uint64_t * search_space = new uint64_t[search_space_size];
		for(uint64_t i = 0; i < search_space_size; i++) {
			search_space[i] = i;
		}
		uint64_t query_count =  2048;
		uint64_t * queries = new uint64_t[query_count];
		uint64_t _seed = seed;
		for(uint i = 0; i < query_count; i++) {
			queries[i] = fast_rand(&_seed) % (search_space_size * 2);
		}
		
		for (int i = 10; i <= 20; i+=5) {
			seed = dist(rd);
			search_space_size = 1 << i;
			
			std::shuffle(search_space, &search_space[search_space_size], fast_URBG(seed));
			auto ref = sum_of_locations(         search_space, search_space_size, queries, query_count);
			auto fut = sum_of_locations_solution(search_space, search_space_size, queries, query_count);
			EXPECT_EQ(ref, fut);
			std::cerr << search_space_size << " " << query_count << " " << ref << "\n";
		}

	}	
			  
}

int main(int argc, char **argv) {
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
