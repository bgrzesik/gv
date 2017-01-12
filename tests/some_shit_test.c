
#include <gv.h>

#define STB_DEFINE
#define STB_IMAGE_IMPLEMENTATION
#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#include <stb.h>
#include <stb_image.h>
#include <stb_easy_font.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>

#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32s.lib")

struct Vertex {
    union { 
        struct { float x, y; };
        float pos[2];
    };

    union { 
        struct { float r, g, b, a; };
        float col[4];
    };

    union { 
        struct { float u, v; };
        float uv[2];
    };
};

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

    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
        return 3;
    }

    static const struct Vertex vertices[] = {
        {  1.0f,  1.0f,  1.0f, 1.0f, 1.0f, 1.0f,  1.0f, 0.0f },
        { -1.0f,  1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f },
        { -1.0f, -1.0f,  0.0f, 1.0f, 1.0f, 1.0f,  0.0f, 1.0f },
        {  1.0f, -1.0f,  1.0f, 1.0f, 0.0f, 1.0f,  1.0f, 1.0f },
    };

    unsigned short indices[] = {
        0, 1, 2,
        0, 2, 3,
    };

    int w, h, n;
    unsigned char *data = stbi_load("../tests/avatar.jpg", &w, &h, &n, 4);

    if (!data) {
        fprintf(stderr, "cannot load texture 'tests/avatar.jpg'\n");
        goto load_tex_failed;
    }

    glEnable(GL_TEXTURE_2D);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        int win_w, win_h;
        glfwGetWindowSize(window, &win_w, &win_h);

        float ratio_horiz = gvMaxf(win_w / (float) win_h, 1.0f);
        float ratio_verti = gvMaxf(win_h / (float) win_w, 1.0f);

        glViewport(0, 0, win_w, win_h);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(-ratio_horiz, ratio_horiz, -ratio_verti, ratio_verti, -1.0f, 1.0f);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        glClientActiveTexture(GL_TEXTURE0);

        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_COLOR_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);
        
        glVertexPointer(2, GL_FLOAT, sizeof(vertices[0]), &vertices[0].x);
        glColorPointer(4, GL_FLOAT, sizeof(vertices[0]), &vertices[0].r);
        glTexCoordPointer(2, GL_FLOAT, sizeof(vertices[0]), &vertices[0].u);
    
        glBindTexture(GL_TEXTURE_2D, tex);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

        glDisableClientState(GL_VERTEX_ARRAY);
        glDisableClientState(GL_COLOR_ARRAY);
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);

        static char buffer[99999];
        int num_quads;

        float scale = 50.0f;

        num_quads = stb_easy_font_print(-scale, -scale, "THIS SHIT WORKS!!!", NULL, buffer, sizeof(buffer));

        glScalef(1.0f / scale, -1.0f / scale, 1.0f);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(2, GL_FLOAT, 16, buffer);

        glBindTexture(GL_TEXTURE_2D, 0);
        glDrawArrays(GL_QUADS, 0, num_quads * 4);

        glDisableClientState(GL_VERTEX_ARRAY);

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    free(data);

load_tex_failed:
    glfwDestroyWindow(window);
    glfwTerminate();
}