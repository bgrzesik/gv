/*
	gv.h - private code - Bartlomiej's Utilities


ABOUT:
	This is simple utilities for code writing


 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GV_H__
#define __GV_H__

#if !defined(__GNUC__) && !(defined(__llvm__) || defined(__clang__))
#error "GNU compiler and clang is only supported at the moment"
#endif

#if (defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)) && !defined(GV_SYS_64BIT)
#define GV_SYS_64BIT
#elif !defined(GV_SYS_32BIT)
#define GV_SYS_32BIT
#else
#error "Unknown architecture"
#endif

#if defined(__llvm__) || defined(__clang__)
#define gv__pragma(...) _Pragma(#__VA_ARGS__)
#else
#define gv__pragma(...)
#endif

#ifndef GV_INLINE
#define GV_INLINE inline
#endif

#ifndef GV_PACKED
#define GV_PACKED __attribute__((__packed__))
#endif

#ifndef GV_ASSERT
#include <assert.h>
#define GV_ASSERT(...) assert(__VA_ARGS__)
#endif

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ > 19901L)
#include <stdint.h>

#ifndef gvu8_t
#define gvu8_t uint8_t
#endif

#ifndef gvu16_t
#define gvu16_t uint16_t
#endif

#ifndef gvu32_t
#define gvu32_t uint32_t
#endif

#ifndef gvu64_t
#define gvu64_t uint64_t
#endif

#ifndef gvs8_t
#define gvs8_t int8_t
#endif

#ifndef gvs16_t
#define gvs16_t int16_t
#endif

#ifndef gvs32_t
#define gvs32_t int32_t
#endif

#ifndef gvs64_t
#define gvs64_t int64_t
#endif

#ifndef gvintptr_t
#define gvintptr_t intptr_t
#endif

#ifndef gvptrdiff_t
#define gvptrdiff_t ptrdiff_t
#endif

#ifndef gvsize_t
#define gvsize_t size_t
#endif

#else /* __STDC_VERSION__ > 19901L */

#ifndef gvu8_t
#define gvu8_t unsigned char
#endif

#ifndef gvu16_t
#define gvu16_t unsigned short
#endif

#ifndef gvu32_t
#define gvu32_t unsigned int
#endif

#ifndef gvu64_t
#define gvu64_t unsigned long long
#endif

#ifndef gvs8_t
#define gvs8_t char
#endif

#ifndef gvs16_t
#define gvs16_t short
#endif

#ifndef gvs32_t
#define gvs32_t int
#endif

#ifndef gvs64_t
#define gvs64_t long long
#endif

#ifdef GV_SYS_64BIT

#ifndef gvintptr_t
#define gvintptr_t gvu64_t 
#endif

#ifndef gvptrdiff_t
#define gvptrdiff_t gvu64_t
#endif

#else /* GV_SYS_64BIT */

#ifndef gvintptr_t
#define gvintptr_t gvu32_t
#endif

#ifndef gvptrdiff_t
#define gvptrdiff_t gvu32_t
#endif

#endif /* GV_SYS_64BIT */

#define gvsize_t gvintptr_t

#endif /* __STDC_VERSION__ > 19901L */


#ifndef gvmin
#define gvmin(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a < _b ? _a : _b); })
#endif

#ifndef gvmax
#define gvmax(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a > _b ? _a : _b); })
#endif

#ifndef GV_STATIC_ASSERT
#if defined(__has_feature) && __has_feature(c_static_assert)
#define GV_STATIC_ASSERT(...) _Static_assert(!!(__VA_ARGS__))
#else
#define GV_STATIC_ASSERT(...) struct { int: (!!(__VA_ARGS__)); }
#endif
#endif

#ifdef _WIN32
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#ifndef gvdl_open
#define gvdl_open(lib) LoadLibrary((lib));
#endif

#ifndef gvdl_symbol
#define gvdl_symbol(lib, sym) ((void *) GetProcAddress((lib), (sym)))
#endif

#ifdef gvdl_close
#define gvdl_close(lib) FreeLibrary((lib))
#endif

#ifndef gvsock_init
#define gvsock_init() ({ WSADATA wsa_data; WSAStartup(MAKEWORD(2, 2), &wsa_data); })
#endif

#ifndef gvsock_cleanup
#define gvsock_cleanup() WSACleanup()
#endif

#ifndef gvsock_close
#define gvsock_close(sock) closesocket(sock) 
#endif

#ifndef GVSOCK_INVALID
#define GVSOCK_INVALID INVALID_SOCKET
#endif

#ifndef GVSOCK_ERROR
#define GVSOCK_ERROR SOCKET_ERROR
#endif

#ifndef GVSOCK_SD_RECV
#define GVSOCK_SD_RECV SD_RECEIVE
#endif

#ifndef GVSOCK_SD_SEND
#define GVSOCK_SD_SEND SD_SEND
#endif

#ifndef GVSOCK_SD_BOTH
#define GVSOCK_SD_BOTH SD_BOTH
#endif

#ifndef gvsock_t
#define gvsock_t SOCKET
#endif

#ifndef gvthread_start
#define gvthread_start(func, param) CreateThread(NULL, 0, (DWORD (*)(void*))func, param, 0, NULL)
#endif

#ifndef gvthread_join
#define gvthread_join(thread) WaitForSingleObject((thread), INFINITE)
#endif

#ifndef gvthread_t
#define gvthread_t HANDLE
#endif

#else /* _WIN32 */

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#ifndef gvdl_open
#define gvdl_open(lib) dlopen((lib), RTLD_LAZY)
#endif

#ifndef gvdl_symbol
#define gvdl_symbol(lib, sym) dlsym((lib), (sym))
#endif

#ifdef gvdl_close
#define gvdl_close(lib) dlclose((lib))
#endif

#ifndef gvsock_init
#define gvsock_init() do {} while(0)
#endif

#ifndef gvsock_cleanup
#define gvsock_cleanup() do {} while(0)
#endif

#ifndef gvsock_close
#define gvsock_close(sock) close(sock) 
#endif

#ifndef GVSOCK_INVALID
#define GVSOCK_INVALID (-1)
#endif

#ifndef GVSOCK_ERROR
#define GVSOCK_ERROR (-1)
#endif

#ifndef GVSOCK_SD_RECV
#define GVSOCK_SD_RECV SHUT_RD
#endif

#ifndef GVSOCK_SD_SEND
#define GVSOCK_SD_SEND SHUT_WR
#endif

#ifndef GVSOCK_SD_BOTH
#define GVSOCK_SD_BOTH SHUT_RDWR
#endif

#ifndef gvsock_t
#define gvsock_t int
#endif

#ifndef gvthread_start
#define gvthread_start(func, param) 															\
		({																						\
			pthread_t _th = NULL;																\
			gv__pragma(clang diagnostic push)													\
			gv__pragma(clang diagnostic ignored "-Wincompatible-pointer-types")					\
		 	void (*_func) = (func);																\
			gv__pragma(clang diagnostic pop)													\
			pthread_create(&_th, NULL, _func, param); 											\
		 	_th;																				\
		})																						
#endif

#ifndef gvthread_join
#define gvthread_join(thread) pthread_join((thread), NULL)
#endif

#ifndef gvthread_t
#define gvthread_t pthread_t
#endif

#endif /* _WIN32 */

#ifndef gvmem_memset
#include <string.h>
#define gvmem_memset(ptr, val, size) memset(ptr, val, size)
#endif

#ifndef gvdebug_log
#include <stdio.h>
#define gvdebug_log(...) printf(__VA_ARGS__);
#endif

extern GV_INLINE void gvmem_init();

extern GV_INLINE void *gvmem_malloc(gvsize_t size);

extern GV_INLINE void *gvmem_calloc(gvsize_t num, gvsize_t size);

extern GV_INLINE void *gvmem_realloc(void *ptr, gvsize_t size);

extern GV_INLINE void gvmem_free(void *ptr);


#ifdef GV_USE_MEM_FUNCS

#undef malloc
#undef calloc
#undef realloc
#undef free

#define malloc(size) gvmem_malloc(size)
#define calloc(num, size) gvmem_calloc(num, size)
#define realloc(ptr, size) gvmem_realloc(ptr, size)
#define free(ptr) gvmem_free(ptr)

#endif /* GV_USE_MEM_FUNCS */ 


#endif /* __GV_H__ */

/*
	IMPLEMENTATION
 */
#ifdef GV_IMPLEMENTATION

#ifdef _WIN32

HANDLE gv__process_heap = NULL;

GV_INLINE void gvmem_init()
{
	gv__process_heap = GetProcessHeap();
}

GV_INLINE void *gvmem_malloc(gvsize_t size)
{
	return HeapAlloc(gv__process_heap, 0, size);
}

GV_INLINE void *gvmem_calloc(gvsize_t num, gvsize_t size)
{
	return HeapAlloc(gv__process_heap, HEAP_ZERO_MEMORY, size);
}

GV_INLINE void *gvmem_realloc(void *ptr, gvsize_t size)
{
	return HeapReAlloc(gv__process_heap, 0, ptr, size);
}

GV_INLINE void gvmem_free(void *ptr)
{
	return HeapFree(gv__process_heap, 0, ptr);
}

#else /* _WIN32 */

struct gv__mem_block {
	gvsize_t size;
	struct gv__mem_block *next;
	int free : 1;
};

struct gv__mem_block *gv__mem_base = NULL;

struct gv__mem_block *gv__mem_find_free(struct gv__mem_block **last, gvsize_t size)
{
	struct gv__mem_block *curr = gv__mem_base;
	struct gv__mem_block *begining_last = NULL;
	struct gv__mem_block *begining = NULL;
	gvsize_t space_size = 0;

	*last = curr;
	
	gvdebug_log("[MEM] Looking for free memory\n");

	while (curr) {
		if (curr->free && !begining) {  /* maybe begining of a free fragmented space */
			begining = curr;
			begining_last = *last;
			space_size = curr->size;

			gvdebug_log("[MEM] Found free block that may become larger\n");

		} else if (curr->free && begining) {
			space_size += curr->size + sizeof(struct gv__mem_block);	

			gvdebug_log("[MEM] Found next free block that may become a member of bigger block\n");

			if (space_size >= size) { /* found free fragmented space */
				*last = begining_last;
				begining->next = curr->next;
				begining->size = space_size;
				
				gvdebug_log("[MEM] Those found block are big enought to become one big block\n");
				
				return begining;
			}


		} else {
			gvdebug_log("[MEM] Found non free block - aborting try of bigger block \n");

			begining = NULL;
		}

		if (curr->free && curr->size >= size) {
			gvdebug_log("[MEM] Found free block that is big enought for our needs \n");
			break;
		}

		*last = curr;
		curr = curr->next;
	}

	return curr;
}

struct gv__mem_block *gv__mem_req(struct gv__mem_block *last, gvsize_t size)
{
	struct gv__mem_block *block = sbrk(0);
	void *req = sbrk(size + sizeof(struct gv__mem_block));
	GV_ASSERT((void *) block == req);
	
	if (req == (void *) -1) {
		return NULL;
	}
	
	if (last) {
		last->next = block;
	}
	
	block->size = size;
	block->next = NULL;
	block->free = 0;

	gvdebug_log("[MEM] Allocated a new block \n");
	
	return block;
} 

GV_INLINE void gvmem_init()
{
}

GV_INLINE void *gvmem_malloc(gvsize_t size)
{
	struct gv__mem_block *block;
	if (size == 0) {
		return NULL;
	}

	if (!gv__mem_base) { /* this is first call of malloc */
		gvdebug_log("[MEM] First call of malloc allocating memory\n");

		block = gv__mem_req(NULL, size);
		if (!block) {
			return NULL;
		}
		gv__mem_base = block;
	} else {
		gvdebug_log("[MEM] Next call of malloc\n");

		struct gv__mem_block *last = gv__mem_base;
		block = gv__mem_find_free(&last, size);

		if (!block) {
			gvdebug_log("[MEM] No suitable block for our needs, allocating a new one\n");

			block = gv__mem_req(last, size);
			if (!block) {
				return NULL;
			}
		} else if (block->size - size > sizeof(struct gv__mem_block) && last->next == block) {		/* check if block is worth splitting */
			gvdebug_log("[MEM] Found a memory block that is bigger then we need, splitting it\n");

			/* this can result into memory fragmentation, so gv__mem_find_free has to be able to join free blocks */
			struct gv__mem_block *new_block = (struct gv__mem_block *) ((gvintptr_t) (block + 1)) + block->size;
			new_block->size = block->size - size - sizeof(struct gv__mem_block);
			new_block->next = block;
			new_block->free = 1;
			last->next = new_block;
		}
	}
	block->free = 0;
	gvdebug_log("[MEM] Returing ready memory block %p\n", block + 1);

	return block + 1;
}

GV_INLINE void *gvmem_calloc(gvsize_t num, gvsize_t size)
{
	gvsize_t true_size = num * size;
	void *ptr = malloc(true_size);
	
	gvmem_memset(ptr, 0, true_size);

	return ptr;
}

GV_INLINE void *gvmem_realloc(void *ptr, gvsize_t size)
{
	return NULL;
}

GV_INLINE void gvmem_free(void *ptr)
{
	if (!ptr) {
		return;
	}

	struct gv__mem_block *block_ptr = ((struct gv__mem_block *) ptr) - 1;
	GV_ASSERT(block_ptr->free == 0);
	block_ptr->free = 1;

	gvdebug_log("[MEM] Marked memory block as free to use\n");

	if (block_ptr->next == NULL) {
		gvdebug_log("[MEM] Freed block is on the top of the heap, decreasing heap size\n");

		struct gv__mem_block *curr = gv__mem_base;
		struct gv__mem_block *before = NULL;
		struct gv__mem_block *before_begining = NULL;
		struct gv__mem_block *begining = NULL;
		
		while (curr) {
			if (curr->free && begining == NULL) { /* maybe a beginging of free space on top of the heap */
				before_begining = before;
				begining = curr;
			}
			
			if (curr->next == NULL) {
				GV_ASSERT(curr == block_ptr);
				gvdebug_log("[MEM] Deallocating a free blocks on top of the heap\n");
				
				if(before_begining) {
					before_begining->next = NULL;
				} else {
					GV_ASSERT(begining == gv__mem_base && curr == block_ptr);
					gv__mem_base = NULL;

					gvdebug_log("[MEM] All blocks on the heap are marked as free, deallocating whole heap\n");
				}

				brk(begining);
				break;
			} else if (!curr->free) { /* we are not yet on top of the heap */
				begining = NULL;
			}
			
			before = curr;
			curr = curr->next;
		}
	}
}

#endif /* _WIN32 */

#endif /* GV_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif


