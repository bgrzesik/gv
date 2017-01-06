
#include <gv.h>

#define STB_DEFINE
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <stb.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif


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

    struct { float x, y; float r, g, b, a; } vertices[] = {
        {  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0f },
        { -0.5f,  0.5f, 1.0f, 0.0f, 1.0f, 1.0f },
        { -0.5f, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f },
        {  0.5f, -0.5f, 1.0f, 1.0f, 0.0f, 1.0f },
    };

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        
        glVertexPointer(2, GL_FLOAT, sizeof(vertices[0]), vertices);
        glColorPointer(4, GL_FLOAT, sizeof(vertices[0]), &vertices[0].r);
        
        glDrawArrays(GL_QUADS, 0, 4);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    nk_free(&ctx);

    glfwDestroyWindow(window);
    glfwTerminate();
}