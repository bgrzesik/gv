
#include <stdlib.h>
#include <stdio.h>

#define GV_IMPLEMENTATION
#include "../gv.h"

GV_CONSTRUCTOR(preMain) {
    printf("preMain()\n");
}

int main(int argc, const char **argv) {
    printf("main()\n");
    return 0;
}
