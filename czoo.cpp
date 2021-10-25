#include <cstdlib>
#include <unistd.h>
#include<cstdint>
#include<map>
#include<algorithm>
#include<vector>

#define CLINK extern "C"
#define OPT(a) __attribute__(a)

#define NO_OPT __attribute__((optimize(0)))
#define OPT4 __attribute__((optimize(4)))

#define START_C extern "C" { // this just hides the braces from the editor, so it won't try to indent everything.

#define END_C  }
START_C

uint64_t * if_ex(uint64_t * array,
		 unsigned long int size) {
	if (size == 0) {
		return NULL;
	}
	return array;
}


uint64_t * if_else_if_else(uint64_t * array,
			   unsigned long int size) {
	if (size/2) {
		return NULL;
	} else if (size/3) {
		return &array[size/3];
	} else {
		return array;
	}
}

uint64_t * loop(uint64_t * array,
		unsigned long int size) {
	for(uint i = 0; i < size; i++) {
		array[i] = 0;
	}
	return array;
}



uint64_t * fill_vector(uint64_t * array,
		       unsigned long int size) {
	std::sort(&array[0], &array[size-1]);

	/*std::vector<uint64_t> t;
	for(uint i = 0; i < size; i++) {
		t.push_back(size*i);
		}*/
	return &array[4];
}

// register assignment
// loop invariant code motion
// strength reduction
// constant propagation
// How many division instructions will this code execute? -- 1!
uint64_t * div_loop(uint64_t * array,
		unsigned long int size) {
#define X 4
#define Y 8
#define DIV(a,b) (a / b)
	
	for(uint i = 0; i < DIV(size, 3); i++) {
		array[DIV(i, 2) +
		      DIV(Y, X)] = DIV(size, 3);
	}
	return array;
}





uint64_t * ifelse_complex(uint64_t * array,
			 unsigned long int size) {
	if (array[size/2]) {
		return new uint64_t;
	} else {
		return new uint64_t[2];
	}
}


uint64_t * switcher(uint64_t * array,
			   unsigned long int size) {
	switch (size) {

	case 0:
		return &array[1];
		break;
	case 1:
		return &array[4];
		break;
	case 2:
		return &array[5];
		break;
	case 3:
		return &array[2];
		break;
	case 4:
		return &array[11];
		break;
	case 5:
		return &array[10];
		break;
	}
	
	return array;
}

uint64_t * loop_func(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	for(uint i = 0; i < size; i++) {
		t = if_else_if_else(array, size);
	}
	return t;
	
}

uint64_t * loop_if(uint64_t * array,
		   unsigned long int size) {
	uint64_t* t= NULL;
	int k = 0;
	for(uint i = 0; i < size; i++) {
		if (i-size != 0) { //  L1
			k = 4; 
		} else if (i+size != 0) { // L2
			k = 5;
		}
	}
	return t + k; // L3
	
}

uint64_t * loop_func2(uint64_t * array,
		      unsigned long int size) {

	uint64_t s = 0;
	for(uint i = 0; i < 10; i++) {
		s += array[i];
	}
	return &array[s];
	
}

uint64_t * __attribute__((optimize("unroll-all-loops"))) loop_complex(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	for(uint i = 0; i < size; i++) {
		t = ifelse_complex(array, size);
	}
	return t;
	
}


uint64_t * __attribute__((optimize(0))) sum_0(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	int s = 0;
	for(uint i = 0; i < size; i++) {
		s += array[i];
	}
	return t + s;
	
}
uint64_t * __attribute__((optimize(1))) sum_1(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	int s = 0;
	for(uint i = 0; i < size; i++) {
		s += array[i];
	}
	return t + s;
	
}
uint64_t * __attribute__((optimize(2))) sum_2(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	int s = 0;
	for(uint i = 0; i < size; i++) {
		s += array[i];
	}
	return t + s;
	
}
uint64_t * __attribute__((optimize(3))) sum_3(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	int s = 0;
	for(uint i = 0; i < size; i++) {
		s += array[i];
	}
	return t + s;
	
}

uint64_t * __attribute__((optimize(3, "unroll-all-loops"))) sum_unroll(uint64_t * array,
			   unsigned long int size) {
	uint64_t* t= NULL;
	int s = 0;
	for(uint i = 0; i < size; i++) {
		s += array[i];
	}
	return t + s;
	
}


	


END_C
std::map<const std::string, std::pair<std::string, void*>> function_map =
{
#define ONE_ARRAY_FUNC(n) {#n, std::pair<std::string, void* >("1_array", (void*)n)}
	ONE_ARRAY_FUNC(if_ex),
	ONE_ARRAY_FUNC(if_else_if_else),
	ONE_ARRAY_FUNC(ifelse_complex),
	ONE_ARRAY_FUNC(loop),
	ONE_ARRAY_FUNC(loop_func),
	ONE_ARRAY_FUNC(loop_func2),
	ONE_ARRAY_FUNC(sum_0),
	ONE_ARRAY_FUNC(sum_1),
	ONE_ARRAY_FUNC(fill_vector),
	ONE_ARRAY_FUNC(sum_2),
	ONE_ARRAY_FUNC(sum_3),
	ONE_ARRAY_FUNC(sum_unroll),
};

