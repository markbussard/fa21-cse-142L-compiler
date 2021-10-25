#include <cstdlib>
#include "archlab.hpp"
#include <unistd.h>
#include"omp.h"
#include"papi.h"
#include<algorithm>
#include<cstdint>
#include<unordered_set>
#include"function_map.hpp"



#define START_C extern "C" { // this just hides the braces from the editor, so it won't try to indent everything.
#define END_C  }


START_C

uint64_t sum_of_locations_solution(uint64_t *search_space, uint32_t search_space_size, uint64_t* queries, uint32_t query_count)
{
	uint64_t r = 0;

	for(uint32_t i = 0; i < query_count; i++) {
		for(uint32_t j = 0; j < search_space_size; j++) {
			if (search_space[j] == queries[i]) {
				r += i;
				break;
			}
		}
	}
	return r;
}

// hello

FUNCTION("sum_impl", sum_of_locations_solution);

END_C
