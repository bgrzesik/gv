/*
    gv.h - private code - Bartlomiej's Utilities


ABOUT:
    This is simple utilities for code writing

TODOS:
    - TODO add more functions to gv socket.
    - TODO consider moving to camel case naming convention.

NOTES:
    In some macros there are sometimes semi-colons (they shouldn't be there),
    this is due to some problems of Visual Studio Code.

AUTHOR:
    Bartłomiej Grzesik

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
#include <WS2tcpip.h>
#include <Windows.h>

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
#if (defined(_WIN64) || defined(__x86_64__) || defined(__ppc64__)) && !defined(GV_SYS_64BIT)
#define GV_SYS_64BIT
#elif !defined(GV_SYS_32BIT) && !defined(GV_SYS_64BIT)
#define GV_SYS_32BIT
#else
#error "Unknown architecture"
#endif

#if defined(__STDC__) && !defined(GV__C89)
#define GV__C89
#endif

#if defined(__STDC_VERSION__)
#if !defined(GV__C94) && __STDC_VERSION__ >= 199409L
#define GV__C94
#endif
#if !defined(GV__C99) && __STDC_VERSION__ >= 199901L
#define GV__C99
#endif
#if !defined(GV__C11) && __STDC_VERSION__ >= 201112L
#define GV__C11
#endif
#endif

#if (defined(__llvm__) || defined(__clang__) || defined(__GNUC__)) && !defined(GV__PRAGMA)
#define GV__PRAGMA(...) _Pragma(#__VA_ARGS__)
#elif defined(_MSC_VER) && !defined(GV__PRAGMA)
#define GV__PRAGMA(...) __pragma(#__VA_ARGS__)
#elif !defined(GV__PRAGMA)
#define GV__PRAGMA(...)
#endif

#if !defined(_MSC_VER) && !defined(GV__DIAG)
#ifdef __GNUC__
#define GV__DIAG(...) GV__PRAGMA(GCC diagnostic __VA_ARGS__)
#else
#define GV__DIAG(...) GV__PRAGMA(clang diagnostic __VA_ARGS__)
#endif
#elif !defined(GV__DIAG)
#define GV__DIAG(...)
#endif

#ifndef GV_API
#ifdef GV_STATIC
#define GV_API static
#else
#define GV_API extern
#endif
#endif

#ifndef GV_FORCE_INLINE
#ifdef _MSC_VER
#define GV_FORCE_INLINE static inline __forceinline
#else
#define GV_FORCE_INLINE static inline __attribute__((always_inline))
#endif
#endif

#ifndef GV_ALIGN
#ifdef _MSC_VER
#define GV_ALIGN(x) __declspec(align(x))
#else
#define GV_ALIGN(x) __attribute__((aligned(x)))
#endif
#endif

#ifndef GV_ASSERT
#include <assert.h>
#define GV_ASSERT(...) assert(__VA_ARGS__)
#endif

#ifndef GV_DEFER
#define GV_DEFER(...) __VA_ARGS__
#endif

#if defined(GV__C11) && !defined(GV_STATIC_ASSERT)
#define GV_STATIC_ASSERT(...) _Static_assert(!!(__VA_ARGS__), #__VA_ARGS__)
#endif

#if defined(__has_feature) && !defined(GV_STATIC_ASSERT)
#if __has_feature(c_static_assert)
#define GV_STATIC_ASSERT(...) _Static_assert(!!(__VA_ARGS__), #__VA_ARGS__)
#endif
#endif

#ifndef GV_STATIC_ASSERT
#define GV_STATIC_ASSERT(...)                       \
        GV__DIAG(push)                              \
        GV__DIAG(ignored "-Wmissing-declarations")  \
        struct { int: (!!(__VA_ARGS__)); };         \
        GV__DIAG(pop)
#endif

#ifndef GV_DEBUG_BREAK
#ifdef _WIN32
#define GV_DEBUG_BREAK() __debugbreak()
#else
#define GV_DEBUG_BREAK() asm("int $3")
#endif
#endif

#if defined(_MSC_VER) && !defined(GV_CONSTRUCTOR)
#ifdef _WIN64
#define GV__CONSTRUCTOR_PREFIX ""
#else
#define GV__CONSTRUCTOR_PREFIX "_"
#endif
#pragma section(".CRT$XCU", read)
#define GV_CONSTRUCTOR(fn)                                                  \
    static void fn(void);                                                   \
    __pragma(section(".CRT$XCU", read))                                     \
    __declspec(allocate(".CRT$XCU")) void (*fn##_f)(void) = &fn;            \
    __pragma(comment(linker, "/include:" GV__CONSTRUCTOR_PREFIX #fn "_f"))  \
    static void fn(void)
#elif !defined(GV_CONSTRUCTOR)  /* defined(_MSC_VER) && !defined(GV_CONSTRUCTOR) */
#define GV_CONSTRUCTOR(fn) static void __attribute__ ((unused)) fn(void)
#endif  /* defined(_MSC_VER) && !defined(GV_CONSTRUCTOR) */


/*
    MEMORY
 */
#ifndef gv_memset
#include <string.h>
#define gv_memset(ptr, val, size) memset(ptr, val, size)
#endif

#ifndef gv_memcpy
#include <string.h>
#define gv_memcpy(dest, src, size) memcpy(dest, src, size)
#endif

#ifdef GV_USE_MEM_FUNCS
#undef memset
#undef memcpy

#define memset(ptr, val, size) gv_memset(ptr, val, size)
#define memcpy(dest, src, size) gv_memcpy(dest, src, size)
#endif


/*
   TYPES
 */
#ifdef GV__C99
#include <stdint.h>

typedef uint8_t             gvu8_t;
typedef uint16_t            gvu16_t;
typedef uint32_t            gvu32_t;
typedef uint64_t            gvu64_t;
typedef int8_t              gvs8_t;
typedef int16_t             gvs16_t;
typedef int32_t             gvs32_t;
typedef int64_t             gvs64_t;
typedef intptr_t            gvintptr_t;
typedef size_t              gvsize_t;
typedef _Bool               gvbool_t;

#else /* __STDC_VERSION__ > 19901L */

typedef unsigned char       gvu8_t;
typedef unsigned short      gvu16_t;
typedef unsigned int        gvu32_t;
typedef unsigned long long  gvu64_t;
typedef char                gvs8_t;
typedef short               gvs16_t;
typedef int                 gvs32_t;
typedef long long           gvs64_t;

#ifdef GV_SYS_64BIT
typedef gvu64_t             gvintptr_t;
#else
typedef gvu32_t             gvintptr_t;
#endif

typedef gvintptr_t          gvsize_t;
typedef int                 gvbool_t;

#endif /* __STDC_VERSION__ > 19901L */

enum {
    GV_FALSE    = 0,
    GV_TRUE     = 1,
};


GV_STATIC_ASSERT(sizeof(gvu8_t) == 1);
GV_STATIC_ASSERT(sizeof(gvu16_t) == 2);
GV_STATIC_ASSERT(sizeof(gvu32_t) == 4);
GV_STATIC_ASSERT(sizeof(gvu64_t) == 8);

GV_STATIC_ASSERT(sizeof(gvs8_t) == 1);
GV_STATIC_ASSERT(sizeof(gvs16_t) == 2);
GV_STATIC_ASSERT(sizeof(gvs32_t) == 4);
GV_STATIC_ASSERT(sizeof(gvs64_t) == 8);

GV_STATIC_ASSERT(sizeof(gvintptr_t) == sizeof(void *));


/*
    MIN MAX
 */
GV_FORCE_INLINE int gv_min(int a, int b) { return a < b ? a : b; }
GV_FORCE_INLINE int gv_max(int a, int b) { return a > b ? a : b; }

GV_FORCE_INLINE float gv_minf(float a, float b) { return a < b ? a : b; }
GV_FORCE_INLINE float gv_maxf(float a, float b) { return a > b ? a : b; }

GV_FORCE_INLINE long gv_minl(long a, long b) { return a < b ? a : b; }
GV_FORCE_INLINE long gv_maxl(long a, long b) { return a > b ? a : b; }

GV_FORCE_INLINE char gv_minc(char a, char b) { return a < b ? a : b; }
GV_FORCE_INLINE char gv_maxc(char a, char b) { return a > b ? a : b; }


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
    GVSOCK_ERROR    = SOCKET_ERROR,
    GVSOCK_SD_RECV  = SD_RECEIVE,
    GVSOCK_SD_SEND  = SD_SEND,
    GVSOCK_SD_BOTH  = SD_BOTH,
};

#else /* _WIN32 */
typedef int gvsock_t;

#ifndef GVSOCK_INVALID
#define GVSOCK_INVALID (-1)
#endif

enum {
    GVSOCK_ERROR    = -1,
    GVSOCK_SD_RECV  = SHUT_RD,
    GVSOCK_SD_SEND  = SHUT_WR,
    GVSOCK_SD_BOTH  = SHUT_RDWR,
};
#endif /* _WIN32 */

GV_API void      gvsock_close(gvsock_t socket);
GV_API gvbool_t  gvsock_init(void);
GV_API void      gvsock_cleanup(void);


/*
    THREADS
 */
#ifdef _WIN32

typedef HANDLE gvthread_t;
typedef DWORD (*gvthread_fn_t)(void *);

#ifndef GV_THREAD_FN
#define GV_THREAD_FN(name, param) DWORD WINAPI name(void *param)
#endif

#ifndef GV_THREAD_RETURN
#define GV_THREAD_RETURN() ExitThread(0); return 0;
#endif

#else /* _WIN32 */

typedef pthread_t gvthread_t;
typedef void *(*gvthread_fn_t)(void *);

#ifndef GV_THREAD_FN
#define GV_THREAD_FN(name, param) void *name(void *param)
#endif

#ifndef GV_THREAD_RETURN
#define GV_THREAD_RETURN() pthread_exit(0); return NULL;
#endif

#endif

GV_API gvbool_t      gvthread_init(gvthread_t *th, gvthread_fn_t fn, void *param);
GV_API gvbool_t      gvthread_join(gvthread_t *th);


/*
    MUTEX
 */
#ifdef _WIN32
typedef CRITICAL_SECTION gvmutex_t;
#else
typedef pthread_mutex_t gvmutex_t;
#endif

GV_API void      gvmutex_init(gvmutex_t *mutex);
GV_API void      gvmutex_destroy(gvmutex_t *mutex);
GV_API void      gvmutex_lock(gvmutex_t *mutex);
GV_API void      gvmutex_unlock(gvmutex_t *mutex);
GV_API gvbool_t  gvmutex_trylock(gvmutex_t *mutex);


/*
    ATOMIC
 */
GV_API long  gvatomic_cmp_xchg(volatile long *dst, long xchg, long cmp);
GV_API long  gvatomic_xchg_add(volatile long *dst, long val);
GV_API void *gvatomic_cmp_xchgv(void *volatile *dst, void *const xchg, void *const cmp);


/*
    CONDITION VARIABLE
 */
#ifdef _WIN32
typedef CONDITION_VARIABLE gvcondvar_t;
#else
typedef pthread_cond_t gvcondvar_t;
#endif

GV_API gvbool_t  gvcondvar_init(gvcondvar_t *cond);
GV_API gvbool_t  gvcondvar_destroy(gvcondvar_t *cond);
GV_API gvbool_t  gvcondvar_wait(gvcondvar_t *cond, gvmutex_t *mutex);
GV_API gvbool_t  gvcondvar_notify(gvcondvar_t *cond);
GV_API gvbool_t  gvcondvar_notify_all(gvcondvar_t *cond);


/*
    EVENT
 */
#ifdef _WIN32
struct gvevent {
    HANDLE event;
    volatile long waiters;
};
#else   /* _WIN32 */
struct gvevent {
    pthread_cond_t cond;
    pthread_mutex_t mut;
};
#endif  /* _WIN32 */

GV_API gvbool_t  gvevent_init(struct gvevent *event);
GV_API gvbool_t  gvevent_wait(struct gvevent *event);
GV_API gvbool_t  gvevent_notify(struct gvevent *event);
GV_API gvbool_t  gvevent_destroy(struct gvevent *event);


/*
    THREAD POOL
 */
struct gvthread_task {
    gvthread_fn_t            fn;
    void                    *param;
    struct gvthread_task    *next;
};

struct gvthread_pool {
    struct gvthread_task *volatile front;
    struct gvthread_task *volatile rear;

    gvcondvar_t supervisor;
    gvmutex_t guard;

    gvbool_t is_working;
    int num_workers;
    volatile long num_working;

    struct gvevent ready_done;

    gvthread_t *workers;
};

#if defined(_WIN32) && !defined(GV_THREAD_POOL_TASK_RETURN)
#define GV_THREAD_POOL_TASK_RETURN() return 0;
#elif !defined(GV_THREAD_POOL_TASK_RETURN)
#define GV_THREAD_POOL_TASK_RETURN() return NULL;
#endif

GV_API void gvthread_pool_init(struct gvthread_pool *pool, gvsize_t num_workers, gvthread_t *workers);
GV_API void gvthread_pool_schedule(struct gvthread_pool *pool, struct gvthread_task *task, gvthread_fn_t func, void *param);
GV_API void gvthread_pool_destroy(struct gvthread_pool *pool);


#endif  /* __GV_H__ */


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

#else   /* _WIN32 */

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

#endif  /* _WIN32 */

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

GV_API gvbool_t gvthread_init(gvthread_t *th, gvthread_fn_t fn, void *param)
{
    *th = CreateThread(NULL, 0, fn, param, 0, NULL);
    return !!th;
}

GV_API gvbool_t gvthread_join(gvthread_t *th)
{
    return WaitForSingleObject(*th, INFINITE);
}

#else   /* _WIN32 */

GV_API gvbool_t gvthread_init(gvthread_t *th, gvthread_fn_t fn, void *param)
{
    return pthread_create(th, NULL, fn, param) == 0;
}

GV_API gvbool_t gvthread_join(gvthread_t *th)
{
    return pthread_join(*th, NULL);
}

#endif  /* _WIN32 */

/*
    MUTEX IMPLEMENTATION
 */
#ifdef _WIN32

GV_API void gvmutex_init(gvmutex_t *mutex)
{
    gv_memset(mutex, 0, sizeof(*mutex));
    InitializeCriticalSection(mutex);
}

GV_API void gvmutex_destroy(gvmutex_t *mutex)
{
    DeleteCriticalSection(mutex);
}

GV_API void gvmutex_lock(gvmutex_t *mutex)
{
    EnterCriticalSection(mutex);
}

GV_API void gvmutex_unlock(gvmutex_t *mutex)
{
    LeaveCriticalSection(mutex);
}

GV_API gvbool_t gvmutex_trylock(gvmutex_t *mutex)
{
    return TryEnterCriticalSection(mutex);
}

#else   /* _WIN32 */

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

GV_API gvbool_t gvmutex_trylock(gvmutex_t *mutex)
{
    return pthread_mutex_unlock(mutex) == 0;
}

#endif  /* _WIN32 */


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
    return _InterlockedExchangeAdd(dst, val);
}

GV_API void *gvatomic_cmp_xchgv(void *volatile *dst, void *const xchg, void *const cmp)
{
    return _InterlockedCompareExchangePointer(dst, xchg, cmp);
}

#else   /* _WIN32 */

GV_API long gvatomic_cmp_xchg(volatile long *dst, long xchg, long cmp)
{
    return __sync_val_compare_and_swap(dst, cmp, xchg);
}

GV_API long gvatomic_xchg_add(volatile long *dst, long val)
{
    return __sync_fetch_and_add (dst, val);
}

GV_API void *gvatomic_cmp_xchgv(void *volatile *dst, void *const xchg, void *const cmp)
{
    return __sync_val_compare_and_swap(dst, cmp, xchg);
}

#endif  /* _WIN32 */


/*
    CONDITION VARIABLE IMPLEMENTATION
 */
#ifdef _WIN32

GV_API gvbool_t gvcondvar_init(gvcondvar_t *cond)
{
    InitializeConditionVariable(cond);
    return GV_TRUE;
}

GV_API gvbool_t gvcondvar_destroy(gvcondvar_t *cond)
{
    return GV_TRUE;
}

GV_API gvbool_t gvcondvar_wait(gvcondvar_t *cond, gvmutex_t *mutex)
{
    return SleepConditionVariableCS(cond, mutex, INFINITE);
}

GV_API gvbool_t gvcondvar_notify(gvcondvar_t *cond)
{
    WakeConditionVariable(cond);
    return GV_TRUE;
}

GV_API gvbool_t gvcondvar_notify_all(gvcondvar_t *cond)
{
    WakeAllConditionVariable(cond);
    return GV_TRUE;
}

#else   /* _WIN32 */

GV_API gvbool_t gvcondvar_init(gvcondvar_t *cond)
{
    return pthread_cond_init(cond, NULL) == 0;
}

GV_API gvbool_t gvcondvar_destroy(gvcondvar_t *cond)
{
    return pthread_cond_destroy(cond) == 0;
}

GV_API gvbool_t gvcondvar_wait(gvcondvar_t *cond, gvmutex_t *mutex)
{
    return pthread_cond_wait(cond, mutex) == 0;
}

GV_API gvbool_t gvcondvar_notify(gvcondvar_t *cond)
{
    return pthread_cond_signal(cond) == 0;
}

GV_API gvbool_t gvcondvar_notify_all(gvcondvar_t *cond)
{
    return pthread_cond_broadcast(cond) == 0;
}

#endif  /* _WIN32 */


/*
    EVENT IMPLEMENTATION
 */
#ifdef _WIN32

GV_API gvbool_t  gvevent_init(struct gvevent *event)
{
    event->waiters = 0;
    return (event->event = CreateEvent(NULL, TRUE, FALSE, NULL)) != NULL;
}

GV_API gvbool_t  gvevent_wait(struct gvevent *event)
{
    long last;

    _InterlockedExchangeAdd(&event->waiters, 1);
    WaitForSingleObject(event->event, INFINITE);
    last = _InterlockedExchangeAdd(&event->waiters, -1);

    if (last == 1) {
        ResetEvent(event->event);
    }

    return GV_TRUE;
}

GV_API gvbool_t  gvevent_notify(struct gvevent *event)
{
    return SetEvent(event->event);
}

GV_API gvbool_t  gvevent_destroy(struct gvevent *event)
{
    return CloseHandle(event->event);
}


#else   /* _WIN32 */

GV_API gvbool_t  gvevent_init(struct gvevent *event)
{
    int res = 0;
    res |= pthread_mutex_init(&event->mut, NULL);
    res |= pthread_cond_init(&event->cond, NULL);
    return res;
}

GV_API gvbool_t  gvevent_wait(struct gvevent *event)
{
    int res = 0;
    res |= pthread_mutex_lock(&event->mut);
    res |= pthread_cond_wait(&event->cond, &event->mut);
    res |= pthread_mutex_unlock(&event->mut);
    return res == 0;
}

GV_API gvbool_t  gvevent_notify(struct gvevent *event)
{
    int res = 0;
    res |= pthread_mutex_lock(&event->mut);
    res |= pthread_cond_broadcast(&event->cond);
    res |= pthread_mutex_unlock(&event->mut);
    return res == 0;
}

GV_API gvbool_t  gvevent_destroy(struct gvevent *event)
{
    int res = 0;
    res |= pthread_mutex_destroy(&event->mut);
    res |= pthread_cond_destroy(&event->cond);
    return res == 0;
}

#endif  /* _WIN32 */


/*
    THREAD POOL IMPLEMENTATION
 */
GV_THREAD_FN(gvthread_pool__func, param)
{
    struct gvthread_pool *pool = param;
    struct gvthread_task *task = NULL, *last_front = NULL, *last_next = NULL;

    gvatomic_xchg_add(&pool->num_working, 1);

    if (pool->num_working == pool->num_workers) {
        gvevent_notify(&pool->ready_done);
    }

    while (1) {
        gvmutex_lock(&pool->guard);
        while (pool->is_working && !pool->front) {
            gvcondvar_wait(&pool->supervisor, &pool->guard);
        }
        gvmutex_unlock(&pool->guard);

        if (!pool->front) {
            if (!pool->is_working) {
                if (gvatomic_xchg_add(&pool->num_working, -1) == 1) { /* if i'm the last worker notify the destructor */
                    gvevent_notify(&pool->ready_done);
                }

                GV_THREAD_RETURN();
            }
            continue;
        }

        do {
            last_front = pool->front;
            if (last_front) {
                last_next = last_front->next;
                task = gvatomic_cmp_xchgv((void * volatile *) &pool->front, last_next, last_front);
            }
        } while (task != last_front && pool->front != NULL);

        if (task) {
            (*task->fn)(task->param);
        }
    }

    GV_ASSERT(GV_FALSE);
    GV_THREAD_RETURN();
}

GV_API void gvthread_pool_init(struct gvthread_pool *pool, gvsize_t num_workers, gvthread_t*workers)
{
    gv_memset(pool, 0, sizeof(*pool)); /* undefined behavior happens without this */

    pool->rear = NULL;
    pool->front = NULL;
    pool->is_working = GV_TRUE;
    pool->num_workers = num_workers;
    pool->workers = workers;

    gvmutex_init(&pool->guard);
    gvcondvar_init(&pool->supervisor);
    gvevent_init(&pool->ready_done);

    int i;
    for (i = 0; i < pool->num_workers; i++) {
        gvthread_init(&pool->workers[i], &gvthread_pool__func, pool);
    }

    gvevent_wait(&pool->ready_done);
}

GV_API void gvthread_pool_schedule(struct gvthread_pool *pool, struct gvthread_task *task, gvthread_fn_t fn, void *param)
{
    task->fn = fn;
    task->param = param;
    task->next = NULL;

    if (pool->front) {
        pool->rear->next = task;
        pool->rear = task;
    } else {
        pool->front = task;
        pool->rear = task;
    }

    gvmutex_lock(&pool->guard);
    gvcondvar_notify(&pool->supervisor);
    gvmutex_unlock(&pool->guard);
}

GV_API void gvthread_pool_destroy(struct gvthread_pool *pool)
{
    pool->is_working = 0;
    gvmutex_lock(&pool->guard);
    gvcondvar_notify_all(&pool->supervisor);
    gvmutex_unlock(&pool->guard);

    if (pool->num_working > 0) {
        gvevent_wait(&pool->ready_done);
    }
    
    int i;
    for (i = 0; i < pool->num_workers; i++) {
        gvthread_join(&pool->workers[i]);
    }

    gvmutex_destroy(&pool->guard);
    gvcondvar_destroy(&pool->supervisor);
    gvevent_destroy(&pool->ready_done);
}


#endif /* GV_IMPLEMENTATION */


#ifdef __cplusplus
}
#endif


