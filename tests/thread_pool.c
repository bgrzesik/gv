
#include <stdlib.h>
#include <stdio.h>

#define GV_IMPLEMENTATION
#include "gv.h"

volatile long a = 0;

static void *task_func(void *param) 
{
    gvatomic_xchg_add(&a, 1);
    return NULL;
}

int main(int argc, const char **argv)
{
    struct gvthread_pool pool;
    struct gvthread_job workers[4];
    gvthread_pool_init(&pool, 4, workers);

    struct gvthread_task tasks[256];
    int i;
    for (i = 0; i < 256; i++) {
        gvthread_pool_schedule(&pool, &tasks[i], &task_func, (void *)(gvintptr_t) i);
    }

    gvthread_pool_destroy(&pool);
    return 0;
}
