
#define STB_DEFINE
#include <stb.h>

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_FIXED_TYPES
#define NK_IMPLEMENTATION
#include <nuklear.h>

#include <GLFW/glfw3.h>
#include <stdio.h>


void error_handler(int error, const char *desc)
{
    stb_fatal("glfw error %d: %s\n", error, desc);
}

int main(int argc, const char **argv)
{
    GLFWwindow *window;

    if (!glfwInit()) {
        stb_fatal("glfwInit() failed\n");
        return 1;
    }

    window = glfwCreateWindow(1280, 720, "", NULL, NULL);

    if (!window) {
        stb_fatal("glfwCreateWindow() failed\n");
        return 2;
    }

    glfwMakeContextCurrent(window);

    struct nk_context ctx;
    nk_init_default(&ctx, NULL);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    nk_free(&ctx);

    glfwDestroyWindow(window);
    glfwTerminate();
}