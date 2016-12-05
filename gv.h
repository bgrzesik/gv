/*
	gv.h - private code - Bartlomiej's Utilities


ABOUT:
	This is simple utilities for code writing

TODOS:

NOTES:
	- In some macros there are sometimes semi-colons, this is due to some problems of Visual Studio Code. 

 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GV_H__
#define __GV_H__

#ifdef _WIN32

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#include <WinSock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#else /* _WIN32 */

#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <pthread.h>

#endif /* _WIN32 */


/*
   COMPILE TIME
 */
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

#if (defined(__llvm__) || defined(__clang__) || defined(__GNUC__)) && !defined(GV__PRAGMA)
#define GV__PRAGMA(...) _Pragma(#__VA_ARGS__) 
#elif !defined(GV__PRAGMA)
#define GV__PRAGMA(...)
#endif

#ifndef GV_API
#define GV_API static
#endif

#ifndef GV_INLINE
#define GV_INLINE inline
#endif

#ifndef GV_PACKED
#define GV_PACKED __attribute__((__packed__))
#endif

#ifndef GV_ALIGN
#define GV_ALIGN(x) __attribute__((aligned(x)))
#endif

#ifndef GV_ASSERT
#include <assert.h>
#define GV_ASSERT(...) assert(__VA_ARGS__)
#endif

#ifndef GV_DEFER
#define GV_DEFER(...) __VA_ARGS__
#endif

#ifndef GV_STATIC_ASSERT
#if defined(__has_feature) && __has_feature(c_static_assert)
#define GV_STATIC_ASSERT(...) _Static_assert(!!(__VA_ARGS__))
#else
#define GV_STATIC_ASSERT(...) 											\
		GV__PRAGMA(clang diagnostic push)								\
		GV__PRAGMA(clang diagnostic ignored "-Wmissing-declarations")	\
		struct { int: (!!(__VA_ARGS__)); }								\
		GV__PRAGMA(clang diagnostic pop)
#endif
#endif

#ifndef GV_DEBUG_BREAK
#ifdef _WIN32
#define GV_DEBUG_BREAK() __debugbreak()
#else
#define GV_DEBUG_BREAK() asm("int $3")
#endif
#endif


/*
	MEMORY
 */
#ifndef gvmem_memset
#include <string.h>
#define gvmem_memset(ptr, val, size) memset(ptr, val, size)
#endif

#ifndef gvmem_memcpy
#include <string.h>
#define gvmem_memcpy(dest, src, size) memcpy(dest, src, size)
#endif

#ifdef GV_USE_MEM_FUNCS
#undef memset
#undef memcpy

#define memset(ptr, val, size) gvmem_memset(ptr, val, size)
#define memcpy(dest, src, size) gvmem_memcpy(dest, src, size)
#endif


/*
   TYPES
 */
#if defined(__STDC_VERSION__) && (__STDC_VERSION__ > 199901L)
#include <stdint.h>

typedef uint8_t				gvu8_t;
typedef uint16_t			gvu16_t;
typedef uint32_t			gvu32_t;
typedef uint64_t			gvu64_t;
typedef int8_t				gvs8_t;
typedef int16_t				gvs16_t;
typedef int32_t				gvs32_t;
typedef int64_t				gvs64_t;
typedef intptr_t			gvintptr_t;
typedef size_t				gvsize_t;
typedef _Bool				gvbool_t;

#else /* __STDC_VERSION__ > 19901L */

typedef unsigned char		gvu8_t;
typedef unsigned short		gvu16_t;
typedef unsigned int		gvu32_t;
typedef unsigned long long	gvu64_t;
typedef char				gvs8_t;
typedef short				gvs16_t;
typedef int					gvs32_t;
typedef long long			gvs64_t;

#ifdef GV_SYS_64BIT
typedef gvu64_t 			gvintptr_t;
#else
typedef gvu64_t 			gvintptr_t;
#endif

typedef gvintptr_t			gvsize_t;
typedef int					gvbool_t;

#endif /* __STDC_VERSION__ > 19901L */

enum {
	GV_FALSE	= 0,
	GV_TRUE		= 1,
};


GV_STATIC_ASSERT(sizeof(gvu8_t) == 1);
GV_STATIC_ASSERT(sizeof(gvu16_t) == 2);
GV_STATIC_ASSERT(sizeof(gvu32_t) == 4);
GV_STATIC_ASSERT(sizeof(gvu64_t) == 8);

GV_STATIC_ASSERT(sizeof(gvs8_t) == 1);
GV_STATIC_ASSERT(sizeof(gvs16_t) == 2);
GV_STATIC_ASSERT(sizeof(gvs32_t) == 4);
GV_STATIC_ASSERT(sizeof(gvs64_t) == 8);


#ifndef gvmin
#define gvmin(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a < _b ? _a : _b); })
#endif

#ifndef gvmax
#define gvmax(a, b) ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); (_a > _b ? _a : _b); })
#endif


/*
	DYNAMIC LOADING
 */
GV_API void *gvdl_open(const char *name);
GV_API void *gvdl_symbol(void *lib, const char *symbol_name);
GV_API void  gvdl_close(void *lib);


/*
	SOCKET
 */
#ifdef _WIN32
typedef SOCKET gvsock_t;

#ifndef GVSOCK_INVALID
#define GVSOCK_INVALID INVALID_SOCKET
#endif

enum {
	GVSOCK_ERROR 	= SOCKET_ERROR,
	GVSOCK_SD_RECV 	= SD_RECEIVE,
	GVSOCK_SD_SEND 	= SD_SEND,
	GVSOCK_SD_BOTH 	= SD_BOTH,
};

#else /* _WIN32 */
typedef int gvsock_t;

#ifndef GVSOCK_INVALID
#define GVSOCK_INVALID (-1)
#endif

enum {
	GVSOCK_ERROR 	= -1,
	GVSOCK_SD_RECV	= SHUT_RD,
	GVSOCK_SD_SEND	= SHUT_WR,
	GVSOCK_SD_BOTH	= SHUT_RDWR,
};
#endif /* _WIN32 */

GV_API void 	gvsock_close(gvsock_t socket);
GV_API gvbool_t gvsock_init(void);
GV_API void		gvsock_cleanup(void);


/*
	THREADS
 */
#ifdef _WIN32
typedef HANDLE gvthread_id_t;
#else
typedef pthread_t gvthread_id_t;
#endif

typedef void *(*gvthread_func_t)(void *);

struct gvthread_job {
	gvthread_id_t 	 thread_id;
	gvthread_func_t	 func;
	void 			*param;
	void			*result;
	int				 done : 1;
};

GV_API gvbool_t		gvthread_init(struct gvthread_job *thread, gvthread_func_t func, void *param);
GV_API gvbool_t		gvthread_join(struct gvthread_job *thread);


/*
	MUTEX
 */
#ifdef _WIN32
typedef HANDLE gvmutex_t;
#else
typedef pthread_mutex_t gvmutex_t;
#endif

GV_API void		gvmutex_init(gvmutex_t *mutex);
GV_API void		gvmutex_destroy(gvmutex_t *mutex);
GV_API void		gvmutex_lock(gvmutex_t *mutex);
GV_API void		gvmutex_unlock(gvmutex_t *mutex);
GV_API gvbool_t	gvmutex_trylock(gvmutex_t *mutex);


/*
	ATOMIC
 */
GV_API long gvatomic_cmp_xchg(volatile long *dst, long xchg, long cmp);
GV_API long gvatomic_xchg_add(volatile long *dst, long val);


#endif	/* __GV_H__ */


/*
	IMPLEMENTATION
 */
#ifdef GV_IMPLEMENTATION

/*
	DYNAMIC LOADING IMPLEMENTATION
 */
#ifdef _WIN32

GV_API void *gvdl_open(const char *name)
{
	return LoadLibrary(name);
} 

GV_API void *gvdl_symbol(void *lib, const char *symbol_name)
{
	return (void *) GetProcAddress(lib, symbol_name);
}

GV_API void gvdl_close(void *lib)
{
	FreeLibrary(lib);
} 

#else	/* _WIN32 */

GV_API void *gvdl_open(const char *name)
{
	return dlopen(name, RTLD_LAZY);
}

GV_API void *gvdl_symbol(void *lib, const char *symbol_name)
{
	return dlsym(lib, symbol_name);
}

GV_API void gvdl_close(void *lib)
{
	dlclose(lib);
}

#endif	/* _WIN32 */

/*
	SOCKET IMPLEMENTATION
 */
#ifdef _WIN32

GV_API void gvsock_close(gvsock_t socket)
{
	closesocket(socket);
}

GV_API gvbool_t gvsock_init(void)
{
	WSADATA wsa_data; 
	return WSAStartup(MAKEWORD(2, 2), &wsa_data);
}

GV_API void gvsock_cleanup(void)
{
	WSACleanup();
}

#else /* _WIN32 */

GV_API void gvsock_close(gvsock_t socket)
{
	close(socket);
}

GV_API gvbool_t gvsock_init(void)
{
	return 0;
}

GV_API void gvsock_cleanup(void)
{
}

#endif /* _WIN32 */

/*
	THREAD IMPLEMENTATION
 */
#ifdef _WIN32

static DWORD WINAPI gvthread__func(void *param)
{
	struct gvthread_job *j = param;
	j->result = (*j->func)(j->param);
	j->done = 1;
	return !!j->result;
}

GV_API gvbool_t gvthread_init(struct gvthread_job *job, gvthread_func_t func, void *param)
{
	job->param = param;
	job->func = func;
	job->result = NULL;
	job->done = 0;

	job->thread_id = CreateThread(NULL, 0, &gvthread__func, job, 0, NULL);

	return job->thread_id;
}

GV_API gvbool_t	gvthread_join(struct gvthread_job *job)
{
	return WaitForSingleObject(job->thread_id, INFINITE);
}

#else	/* _WIN32 */

static void *gvthread__func(void *param)
{
	struct gvthread_job *j = param;
	j->result = (*j->func)(j->param);
	j->done = 1;
	pthread_exit(j->result);
	return j->result;
}

GV_API gvbool_t gvthread_init(struct gvthread_job *job, gvthread_func_t func, void *param)
{
	job->param = param;
	job->func = func;
	job->result = NULL;
	job->done = 0;

	int res = pthread_create(&job->thread_id, NULL, &gvthread__func, param);
	return res == 0;					
}

GV_API gvbool_t	gvthread_join(struct gvthread_job *job)
{
	return pthread_join(job->thread_id, NULL);
}

#endif	/* _WIN32 */

/*
	MUTEX IMPLEMENTATION
 */
#ifdef _WIN32

GV_API void gvmutex_init(gvmutex_t *mutex)
{
	*mutex = CreateMutex(NULL, FALSE, NULL);
}

GV_API void gvmutex_destroy(gvmutex_t *mutex)
{
	CloseHandle(*mutex);
}

GV_API void gvmutex_lock(gvmutex_t *mutex)
{
	WaitForSingleObject(*mutex, INFINITE);
}

GV_API void gvmutex_unlock(gvmutex_t *mutex)
{
	ReleaseMutex(*mutex);
}

GV_API gvbool_t	gvmutex_trylock(gvmutex_t *mutex)
{
	return WaitForSingleObject(*mutex, 0) == WAIT_OBJECT_0;
}

#else	/* _WIN32 */

GV_API void gvmutex_init(gvmutex_t *mutex)
{
	pthread_mutex_init(mutex, NULL);
}

GV_API void gvmutex_destroy(gvmutex_t *mutex)
{
	pthread_mutex_destroy(mutex);
}

GV_API void gvmutex_lock(gvmutex_t *mutex)
{
	pthread_mutex_lock(mutex);
}

GV_API void gvmutex_unlock(gvmutex_t *mutex)
{
	pthread_mutex_unlock(mutex);
}

GV_API gvbool_t	gvmutex_trylock(gvmutex_t *mutex)
{
	return pthread_mutex_unlock(mutex) == 0;
}

#endif	/* _WIN32 */


/*
	ATOMIC IMPLEMENTATION
 */
#ifdef _WIN32

GV_API long gvatomic_cmp_xchg(volatile long *dst, long xchg, long cmp)
{
	return _InterlockedCompareExchange(dst, xchg, cmp);
}

GV_API long gvatomic_xchg_add(volatile long *dst, long val) 
{
	return _InterlockedExchangeAdd (dst, val);
}

#else	/* _WIN32 */

GV_API long gvatomic_cmp_xchg(volatile long *dst, long xchg, long cmp)
{
	return __sync_val_compare_and_swap(dst, cmp, xchg);
}

GV_API long gvatomic_xchg_add(volatile long *dst, long val) 
{
	return __sync_add_and_fetch (dst, val);
}

#endif	/* _WIN32 */


#endif /* GV_IMPLEMENTATION */


#ifdef __cplusplus
}
#endif


