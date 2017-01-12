

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <wtypes.h>
#include <gl\GL.h>
#include <gl\GLU.h>
#include <math.h>

#define GV_IMPLEMENTATION
#include <gv.h>


#ifdef _MSC_VER
#pragma warning(push, 0)
#endif
#define STB_DEFINE
#include <stb.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_easy_font.h>
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#pragma comment(lib, "opengl32.lib")


struct Window {
    int visible;
    int should_close;
    int texture;
    int width;
    int height;
};

static void init(struct Window *window) {
    int tex_w, tex_h, tex_comp;
    char *data = stbi_load("avatar.jpg", &tex_w, &tex_h, &tex_comp, 4);
    
    if (!data) {
        stb_fatal("unable to load texture 'avatar.jpg'\n");
    }
    
    glEnable(GL_TEXTURE_2D);

    glGenTextures(1, &window->texture);
    glBindTexture(GL_TEXTURE_2D, window->texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex_w, tex_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

    free(data);
}

static void render(struct Window *window) {
    glClear(GL_COLOR_BUFFER_BIT);

#define ha (16.0f)
#define va (9.0f)

    static const struct { float x, y; float r, g, b, a; float u, v; } vertices[] = {
        {  0.5f,  0.5f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f },
        { -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f },
        { -0.5f, -0.5f,  1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f },
        {  0.5f, -0.5f,  1.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f },
        
        {  ha / 2.0f,  va / 2.0f,  0.0f, 0.0f, 1.0f, 1.0f,  0.0f, 0.0f },
        { -ha / 2.0f,  va / 2.0f,  0.0f, 1.0f, 0.0f, 1.0f,  1.0f, 0.0f },
        { -ha / 2.0f, -va / 2.0f,  1.0f, 0.0f, 0.0f, 1.0f,  1.0f, 1.0f },
        {  ha / 2.0f, -va / 2.0f,  1.0f, 1.0f, 1.0f, 0.0f,  0.0f, 1.0f },
    };

    static unsigned short indices[] = {
        0, 1, 2,
        0, 2, 3,

        4, 5, 6,
        4, 6, 7,
    };

    float hratio = gvMaxf(ha / va, window->width / (float) window->height) * va / 2.0f;
    float vratio = gvMaxf(va / ha, window->height / (float) window->width) * ha / 2.0f;
        
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-hratio, hratio, -vratio, vratio, -1.0f, 1.0f);
    glMatrixMode(GL_MODELVIEW);

#undef ha
#undef va

    if (GetKeyState('W') & 0x8000) glTranslatef(0.0f,  0.0005f, 0.0f);
    if (GetKeyState('S') & 0x8000) glTranslatef(0.0f, -0.0005f, 0.0f);
    if (GetKeyState('A') & 0x8000) glTranslatef(-0.0005f, 0.0f, 0.0f);
    if (GetKeyState('D') & 0x8000) glTranslatef( 0.0005f, 0.0f, 0.0f);

    
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(2, GL_FLOAT, sizeof(vertices[0]), &vertices[0].x);
    glColorPointer(4, GL_FLOAT, sizeof(vertices[0]), &vertices[0].r);
    glTexCoordPointer(2, GL_FLOAT, sizeof(vertices[0]), &vertices[0].u);

    glPushMatrix();
    glLoadIdentity();
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices + 6);
    
    glPopMatrix();
    

    glBindTexture(GL_TEXTURE_2D, window->texture);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    static char str[255] = "THIS SHIT WORKS!";
    static unsigned long long frame = 0;
    frame++;
    stb_snprintf(str, sizeof(str), "FRAME: %llu", frame);

    static char easy_font[9999];
    int num_quads = stb_easy_font_print(0, 0, str, NULL, easy_font, sizeof(easy_font));

    float width = stb_easy_font_width(str);
    float height = stb_easy_font_height(str);

    glPushMatrix();
    glLoadIdentity();
    glTranslatef(-width / height / 2.0f, 4.5f, 0.0f);
    glScalef(1.0f / height, -1.0f / height, 1.0f);

    glVertexPointer(2, GL_FLOAT, 16, easy_font);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glDrawArrays(GL_QUADS, 0, num_quads * 4);

    glPopMatrix();
    glDisableClientState(GL_VERTEX_ARRAY);

    glFlush();
}

static void dispose(struct Window *window) {
    glDeleteTextures(1, &window->texture);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static PAINTSTRUCT ps;
    struct Window *window = (struct Window *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!window) {
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }

    switch (msg) {

        case WM_CLOSE:
            window->should_close = 1;
            PostQuitMessage(0);
            return 0;

        case WM_SYSCOMMAND:
            switch (wparam) {
                case SC_MINIMIZE:
                    window->visible = 0;
                    break;
                case SC_RESTORE:
                    window->visible = 1;
                    break;
                case SC_CLOSE:
                    window->should_close = 1;
                    break;
            }
            break;

        case WM_KEYDOWN:
            if (wparam == VK_ESCAPE) {
                window->should_close = 1;
                return 0;
            }
            break;

        case WM_PAINT:
            render(window);
            BeginPaint(hwnd, &ps);
            EndPaint(hwnd, &ps);
            return 0;

        case WM_SIZE:
            window->width = LOWORD(lparam);
            window->height = HIWORD(lparam);
            glViewport(0, 0, window->width, window->height);
            PostMessage(hwnd, WM_PAINT, 0, 0);
            return 0;

    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
}

int WINAPI WinMain(HINSTANCE hinstance, HINSTANCE prev_instance, char *cmd_line, int cmd_show) {
    const wchar_t *class_name = L"Some window class";

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = class_name;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(0,
                               class_name,
                               L"Some window",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
                               NULL, NULL,
                               hinstance,
                               NULL);
    if (hwnd == NULL) {
        ExitProcess(1);
        return 1;
    }

    int pf;
    HDC hdc;
    HGLRC hrc;
    PIXELFORMATDESCRIPTOR pfd;
 
    hdc = GetDC(hwnd);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize        = sizeof(pfd);
    pfd.nVersion     = 1;
    pfd.dwFlags      = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType   = PFD_TYPE_RGBA;
    pfd.cColorBits   = 32;

    pf = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pf, &pfd);
    DescribePixelFormat(hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    ShowWindow(hwnd, cmd_show);
    UpdateWindow(hwnd);

    struct Window window;
    window.should_close = 0;
    window.visible = 1;
    window.texture = 0;
    window.width = 1280;
    window.height = 720;

    SetWindowLongPtr(hwnd, GWLP_USERDATA, &window);
    
    init(&window);

    MSG msg;
    while (!window.should_close) {
        while(PeekMessage(&msg, hwnd, 0, 0, PM_NOREMOVE)) {
            if(GetMessage(&msg, hwnd, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            } else {
                goto end;
            }
            
        }

        if (window.visible) {
            render(&window);
            SwapBuffers(hdc);
        }
        /* update(); */
    }

end:
    dispose(&window);

    wglMakeCurrent(NULL, NULL);
    ReleaseDC(hdc, hwnd);
    wglDeleteContext(hrc);
    DestroyWindow(hwnd);

    ExitProcess(0);
    return 0;
}

#else   /* _WIN32 */

#include <assert.h>
int main(int argc, char **argv) {
    assert(false && "INSTALL FUCKING WINDOWS FOR NOW (I know it hurts)");
    return -1;
}

#endif  /* _WIN32 */
