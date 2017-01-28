
#define GVVK_IMPLEMENTATION
#include <gvvk.h>


int __stdcall WinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, char *cmd_line, int cmd_show) {
    GvVkContext ctx = {0};
    
    gvvkInitContext(&ctx, hinstance, cmd_show);

    gvvkDestroyContext(&ctx);

    exit(0);
    return 0;
}
