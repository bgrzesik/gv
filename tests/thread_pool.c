
#include <stdlib.h>
#include <stdio.h>

#define GV_IMPLEMENTATION
#include "../gv.h"

volatile long a = 0;

GV_THREAD_FN(taskFn, param) {
    gvAtomicXchgAdd(&a, 1);
    GV_THREAD_POOL_TASK_RETURN();
}

int main(int argc, const char **argv) {
    struct GvThreadPool pool;
    GvThread_t workers[4];
    gvThreadPoolInit(&pool, 4, workers);

    struct GvThreadTask tasks[256];
    int i;
    for (i = 0; i < 256; i++) {
        gvThreadPoolSchedule(&pool, &tasks[i], &taskFn, (void *)(gvintptr_t) i);
    }

    gvThreadPoolDestroy(&pool);
    return 0;
}
