#include"function_map.hpp"
#include<cstdint>

uint64_t sum_of_locations(uint64_t *search_space, uint32_t search_space_size, uint64_t* queries, uint32_t query_count)
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
FUNCTION("sum_impl", sum_of_locations);
