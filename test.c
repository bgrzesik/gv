
// #include <stdlib.h>
// #include <stdio.h>
// #include <time.h>

#define GV_USE_MEM_FUNCS
#define GV_IMPLEMENTATION
#include "gv.h"

#include <x86intrin.h>



int main(int argc, const char **argv)
{
	// __m128 a = _mm_setr_ps(1.0f, 2.0f, 3.0f, 4.0f);
	// __m128 b = _mm_setr_ps(10.0f, 20.0f, 30.0f, 40.0f);
	// __m128 c;
	
	// c = a;
	// printf("_mm_setr_ps:\t%f %f %f %f\n", c[0], c[1], c[2], c[3]);
	

	// c = _mm_div_ss(a, b);
	// printf("_mm_div_ss:\t%f %f %f %f\n", c[0], c[1], c[2], c[3]);
	
	// c = _mm_max_ps(c, b);
	// printf("_mm_max_ps:\t%f %f %f %f\n", c[0], c[1], c[2], c[3]);

	// c = _mm_shuffle_ps(a, b, _MM_SHUFFLE(0, 1, 0, 1));
	// printf("_mm_shuffle_ps:\t%f %f %f %f\n", c[0], c[1], c[2], c[3]);

	__attribute__((aligned(16))) int test[4];
	__m128i a = _mm_set1_epi32(10);
	a = _mm_add_epi32(a, _mm_setr_epi32(1, 2, 3, 4));
	_mm_store_si128((__m128i *) test, a);
	printf("_mm_store_si128: \t%d %d %d %d\n", test[0], test[1], test[2], test[3]);

	// volatile long v = 0;
	// printf("v = %ld\n", v);
	// printf("gvatomic_cmp_xchg(%p, %d, %d) => %ld\n", &v, 1, 0, gvatomic_cmp_xchg(&v, 1, 0));
	// printf("v = %ld\n", v);
	// printf("gvatomic_cmp_xchg(%p, %d, %d) => %ld\n", &v, 1, 0, gvatomic_cmp_xchg(&v, 1, 0));
	// printf("v = %ld\n", v);

	return 0;
}

