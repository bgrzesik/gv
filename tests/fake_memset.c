
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define GV_IMPLEMENTATION
#include "../gv.h"

#ifdef _WIN32
#include <intrin.h>
#else
#include <x86intrin.h>
#endif


// static inline void *__cdecl fake_memset(void *ptr, int val, gvsize_t size)
// {
//     register GvU8_t *sp = ptr;
//     register __m128i val128 = _mm_set1_epi32(val);
//     register GvU8_t val8 = val;

//     while ( size-- && ((gvintptr_t) sp & (gvintptr_t) -16) < ((gvintptr_t) sp)) {
//         *(sp++) = val8;        
//     }

//     while ((size / 16) > 0) {
//         _mm_store_si128((__m128i *) sp, val128);
//         sp += 16;
//         size -= 16;
//     }

//     while (size--) {
//         *(sp++) = val8;
//     }
//     return ptr;
// }

#ifdef _MSC_VER
#define GV_CDECL __cdecl
#else
#define GV_CDECL __attribute__((cdecl))
#endif

static inline void *GV_CDECL fake_memset(void *ptr, int val, size_t size) 
{
// 	int cpu[4] = { 0, 0, 0, 0 };

// #ifdef _WIN32
// 	__cpuidex(cpu, 7, 0);
// #else
// 	__asm__ volatile ("cpuid"	:"=a"(cpu[0]), "=b" (cpu[1]), "=c" (cpu[2]), "=d" (cpu[3])
// 								: "a" (7), "c" (0)
// 								: );
// #endif

// 	if (cpu[1] & (1 << 9)) {
#ifdef _WIN32
			__stosb(ptr, val, size);
#else
			__asm__ volatile("rep stosb"	:
											: "D"(ptr), "a"(val), "c"(size)
											: "%edi", "%ecx");
#endif
	// }
	
	return ptr;
}


typedef void *(*gv__memset_func_t)(void *, int, size_t);
gv__memset_func_t gvset;
/*
TODO Implement
	if (supports erms) gvset = version with stosb
	else if (supports simd) gvset = version with simd
	else  gvset = version with longs
 */

#ifdef TEST_FAKE_MEMSET
#undef memset
#define memset(ptr, val, size) fake_memset((ptr), (val), (size))
#endif

int main(int argc, const char **argv)
{
	int i;
	char mem_block[4096];

#ifdef TEST_FAKE_MEMSET
	printf("Starting... (fake_memset)\n");
#else
	printf("Starting... (memset)\n");
#endif

	clock_t begin = clock();

	for (i = 0; i < 1000000; i++) {
		memset(mem_block, -1, sizeof(mem_block));
	}

	clock_t end = clock();

	printf("Test took %ld cycles\n", end - begin);
	printf("Success: %d\n", mem_block[1024] == -1);


	return 0;
}