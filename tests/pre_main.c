
#include <stdlib.h>
#include <stdio.h>

#define GV_IMPLEMENTATION
#include "gv.h"

GV_CONSTRUCTOR(pre_main) 
{
    printf("pre_main()\n");
}

int main(int argc, const char **argv)
{
    printf("main()\n");

    return 0;
}
