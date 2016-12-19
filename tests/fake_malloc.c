
#define GV_IMPLEMENTATION
#include "../gv.h"

#include <stdlib.h>
#include <stdio.h>
#include <time.h>


#define GV_HEAP_SIZE 8192 * 1024

static gvs8_t heap[GV_HEAP_SIZE];

#define GV_HEAP_START heap[0]
#define GV_HEAP_END heap[GV_HEAP_SIZE - 1]

void *gv_sbrk(gvsize_t size)
{
	static volatile const gvs8_t *heap_ptr;
	const volatile gvs8_t *old_heap_ptr;

	static gvbool_t init_sbrk = GV_FALSE;

	if (!init_sbrk) {
		heap_ptr = &GV_HEAP_START;
		init_sbrk = GV_TRUE;
	}

	old_heap_ptr = heap_ptr;
	
	if  ((heap_ptr + size) > &GV_HEAP_END) {
		return (void *) -1;
	}

	heap_ptr += size;
	return (void *) old_heap_ptr;
}

#ifndef _WIN32
#include <unistd.h>
#define gv_sbrk sbrk
#endif

static inline gvsize_t gv__word_align(gvsize_t size) 
{
	return size + ((sizeof(size_t) - 1) & ~(sizeof(size_t) - 1));
}

typedef struct gv__chunk {
	struct gv__chunk    *next, *prev;
	gvsize_t             size;
	gvbool_t             free;
	void                *data;
} gv__chunk_t;                      /* cause it's 'private' */

static void *gv__malloc_base()
{
	static gv__chunk_t *b = NULL;

	if (!b) {
		b = gv_sbrk(gv__word_align(sizeof(gv__chunk_t)));
		GV_ASSERT(b != (void *) -1);
		
		b->next = NULL;
		b->prev = NULL;
		b->free = 0;
		b->size = 0;
		b->data = NULL;
	}
	
	return b;
}

static gv__chunk_t *gv__malloc_find(gvsize_t s, gv__chunk_t **heap)
{
	register gv__chunk_t *c = gv__malloc_base();
	for(; c && !(c->free && c->size >= s); *heap = c, c = c->next);
	return c;
}

static void gv__malloc_merge_next(gv__chunk_t *c) 
{
	c->size += c->next->size + sizeof(gv__chunk_t);
	c->next = c->next->next;

	if (c->next) {
		c->next->prev = c;
	}
}

static void gv__malloc_split_next(gv__chunk_t *c, gvsize_t size) 
{
	gv__chunk_t *newc = (gv__chunk_t*) ((char*) c + size);
	newc->prev = c;
	newc->next = c->next;
	newc->size = c->size - size;
	newc->free = 1;
	newc->data = newc + 1;
	if (c->next) {
		c->next->prev = newc;
	}
	c->next = newc;
	c->size = size - sizeof(gv__chunk_t);
}

void *fake_malloc(gvsize_t size)
{
	if (!size) return NULL;

	gvsize_t len = gv__word_align(size + sizeof(gv__chunk_t));
	gv__chunk_t *prev = NULL;
	gv__chunk_t *c = gv__malloc_find(size, &prev);

	if (!c) {
		gv__chunk_t *newc = gv_sbrk(len);

		if (newc == (void *) -1) {
			return NULL;
		}

		newc->next = NULL;
		newc->prev = prev;
		newc->size = len - sizeof(gv__chunk_t);
		newc->data = newc + 1;
		prev->next= newc;

		c = newc;
	} else if (len + sizeof(gvsize_t) < c->size) {
		gv__malloc_split_next(c, len);        
	}

	c->free = 0;
	return c->data;
}

void fake_free(void * ptr) {
	if (!ptr || ptr < gv__malloc_base() || ptr > gv_sbrk(0)) return;

	gv__chunk_t *c = (gv__chunk_t *) ptr - 1;

	if (c->data != ptr) return;
	c->free = 1;

	if (c->next && c->next->free) {
		gv__malloc_merge_next(c);
	}

	if (c->prev->free) {
		gv__malloc_merge_next(c = c->prev);
	}

	if (!c->next) {
		c->prev->next = NULL;
		gv_sbrk(- c->size - sizeof(gv__chunk_t));
	}
}

//#define TEST_FAKE_MALLOC
#ifdef TEST_FAKE_MALLOC
#undef malloc
#undef free

#define malloc(size) fake_malloc(size)
#define free(ptr) fake_free(ptr);
#endif
/**
 * Test's shown that default malloc takes 0 cycles, but fake malloc took 9 cycles 
 */
int main(int argc, const char **argv)
{
	long i;

	clock_t start = clock();

#ifdef TEST_FAKE_MALLOC 
	printf("Starting... (fake_malloc)\n");
#else
	printf("Starting... (malloc)\n");
#endif

	for (i = 0; i < 1000; i++) {
		float *block_1 = malloc(sizeof(float) * 4);
		float *block_2 = malloc(sizeof(float) * 8);
		
		block_1[1] = 1.0f;
		block_2[1] = 2.0f;

		float *block_3 = malloc(sizeof(float) * 8);
		
		block_3[3] = 1.0f;

		free(block_1);
		free(block_2);

		float *block_4 = malloc(sizeof(float) * 6);
		
		block_4[2] = 1.0f;
		
		free(block_4);
		
		float *block_5 = malloc(sizeof(float) * 2);		/* should use first block*/
		free(block_3);
	}

	clock_t end = clock();

	printf("This took %ld cycles\n", end - start);

	return 0;


/*    float *test = fake_malloc(4 * sizeof(float));

	test[0] = 1.0f;
	test[1] = 1.0f;
	test[2] = 1.0f;
	test[3] = 1.0f;

	printf("\t%f\t%f\t%f\t%f\n", test[0], test[1], test[2], test[3]);

	fake_free(test);*/
}
