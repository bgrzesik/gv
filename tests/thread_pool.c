
#include <stdlib.h>
#include <stdio.h>

#define GV_IMPLEMENTATION
#include "../gv.h"

volatile long a = 0;

GV_THREAD_FN(task_func, param) 
{
    gvatomic_xchg_add(&a, 1);
    GV_THREAD_POOL_TASK_RETURN();
}

int main(int argc, const char **argv)
{
    struct gvthread_pool pool;
    gvthread_t workers[4];
    gvthread_pool_init(&pool, 4, workers);

    struct gvthread_task tasks[256];
    int i;
    for (i = 0; i < 256; i++) {
        gvthread_pool_schedule(&pool, &tasks[i], &task_func, (void *)(gvintptr_t) i);
    }

    gvthread_pool_destroy(&pool);
    return 0;
}
