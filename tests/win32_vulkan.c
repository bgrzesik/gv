
#define GV_IMPLEMENTATION
#include <gv.h>

#include <wtypes.h>
#include <stdint.h>

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#pragma comment(lib, "vulkan-1.lib")

#pragma warning(push, 0)
#define STB_DEFINE
#include <stb.h>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_easy_font.h>
#pragma warning(pop)

GV_STATIC_ASSERT(NULL == 0);

#define VK_CHECK(...) do { VkResult result = (__VA_ARGS__); if (result != VK_SUCCESS) { __debugbreak(); assert(0 && "vulkan error"); } } while (0)


struct gvWindow {
    HINSTANCE hinstance;
    HWND hwnd;
    int should_close : 1;
    int visible : 1;
    int width;
    int height;
    HDC hdc;
};

struct gvVkContext {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;

#ifdef GV_DEBUG
    VkDebugReportCallbackEXT debug_cb;
#endif

    VkQueue graphics_queue;
    uint32_t graphics_family;

    VkQueue compute_queue;
    uint32_t compute_family;

    VkQueue transfer_queue;
    uint32_t transfer_family;

    VkQueue present_queue;
    uint32_t present_family;
};

struct gvVkDisplay {
    struct gvVkContext *ctx;
    struct gvWindow *window;
    VkSurfaceKHR surface;
  
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buff;
    
    VkSwapchainKHR swapchain;
};


static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static PAINTSTRUCT ps;
    struct gvWindow *window = (struct gvWindow *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

    if (!window) {
        return DefWindowProcA(hwnd, msg, wparam, lparam);
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
//        render(window);
        BeginPaint(hwnd, &ps);
        EndPaint(hwnd, &ps);
        return 0;

    case WM_SIZE:
        window->width = LOWORD(lparam);
        window->height = HIWORD(lparam);
        PostMessageA(hwnd, WM_PAINT, 0, 0);
        return 0;

    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

void gvWindowInit(struct gvWindow *window, HINSTANCE hinstance, int cmd_show) {
    static const char *class_name = "Some window class";
    
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = class_name;

    RegisterClassA(&wc);

    window->hwnd = CreateWindowExA(0,
                               class_name,
                               "Some window",
                               WS_OVERLAPPEDWINDOW,
                               CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720,
                               NULL, NULL,
                               hinstance,
                               NULL);

    if (window->hwnd == NULL) {
        MessageBoxA(NULL, "CreateWindowExA() failed", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    int pf;
    PIXELFORMATDESCRIPTOR pfd;

    window->hdc = GetDC(window->hwnd);

    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    pf = ChoosePixelFormat(window->hdc, &pfd);
    SetPixelFormat(window->hdc, pf, &pfd);
    DescribePixelFormat(window->hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    ShowWindow(window->hwnd, cmd_show);
    UpdateWindow(window->hwnd);

    SetWindowLongPtrA(window->hwnd, GWLP_USERDATA, (LONG_PTR) window);
}

void gvWidnowLoop(struct gvWindow *window) {
    MSG msg;
    while (!window->should_close) {
        while (PeekMessageA(&msg, window->hwnd, 0, 0, PM_NOREMOVE)) {
            if (GetMessageA(&msg, window->hwnd, 0, 0)) {
                TranslateMessage(&msg);
                DispatchMessageA(&msg);
            } else {
                return;
            }

        }

        if (window->visible) {
            //render(&window);
            SwapBuffers(window->hdc);
        }
        /* update(); */
    }
}

void gvWindowDestroy(struct gvWindow *window) {
    DestroyWindow(window->hwnd);
}

void gvVkDisplayInitSuface(struct gvVkDisplay *display, struct gvVkContext *ctx, struct gvWindow *window) {
    VkWin32SurfaceCreateInfoKHR surface_ci = {0};
    surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.hinstance = window->hinstance;
    surface_ci.hwnd = window->hwnd;
    
    VK_CHECK(vkCreateWin32SurfaceKHR(ctx->instance, &surface_ci, NULL, &display->surface));
}

#ifdef GV_DEBUG
PFN_vkCreateDebugReportCallbackEXT fpCreateDebugReportCallback;
PFN_vkDestroyDebugReportCallbackEXT fpDestroyDebugReportCallback;

static VkBool32 VKAPI_PTR debugCallback(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT type, uint64_t obj, size_t loc, int32_t code, const char* prefix, const char* msg, void* ud) {
    OutputDebugStringA("[");
    OutputDebugStringA(prefix);
    OutputDebugStringA("] ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");

    if ((f & VK_DEBUG_REPORT_ERROR_BIT_EXT) == VK_DEBUG_REPORT_ERROR_BIT_EXT) {
        __debugbreak();
    }

    return VK_SUCCESS;
}
#endif  /* GV_DEBUG */

void gvVkContextInit(struct gvVkContext *ctx, struct gvVkDisplay *display, struct gvWindow *window) {
    VkExtensionProperties props[24];
    uint32_t props_count = sizeof(props) / sizeof(props[0]);
    vkEnumerateInstanceExtensionProperties(NULL, &props_count, props);

    VkApplicationInfo app_info = {0};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "Win32 Vulkan";
    app_info.apiVersion = VK_MAKE_VERSION(1, 0, 26);

    VkInstanceCreateInfo instance_ci = {0};
    instance_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instance_ci.pApplicationInfo = &app_info;

    uint32_t i;

#ifdef GV_DEBUG
    static const char *layer_names[] = {
        "VK_LAYER_LUNARG_standard_validation",
    };

    instance_ci.ppEnabledLayerNames = layer_names;
    instance_ci.enabledLayerCount = sizeof(layer_names) / sizeof(layer_names[0]);

    static const char *extenstion_names[] = { 
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,              /* fuck linux compatibility */
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    };
    instance_ci.ppEnabledExtensionNames = extenstion_names;
    instance_ci.enabledExtensionCount = sizeof(extenstion_names) / sizeof(extenstion_names[0]);
#endif

    VK_CHECK(vkCreateInstance(&instance_ci, NULL, &ctx->instance));

    OutputDebugStringA("Created VkInstance\n");
        
#ifdef GV_DEBUG
    fpCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugReportCallbackEXT");
    fpDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugReportCallbackEXT");

    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {0};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    debug_report_ci.pfnCallback = &debugCallback;

    VK_CHECK(fpCreateDebugReportCallback(ctx->instance, &debug_report_ci, NULL, &ctx->debug_cb));
#endif

    VkPhysicalDevice devices[16];
    uint32_t devices_count = sizeof(devices) / sizeof(devices[0]);
    vkEnumeratePhysicalDevices(ctx->instance, &devices_count, devices);

    if (devices_count <= 0) {
        MessageBoxA(NULL, "Your computer doesn't support Vulkan.", "Error!", MB_OK);
        vkDestroyInstance(ctx->instance, NULL);
        ExitProcess(2);
    } else if (devices_count > 1) {
        MessageBoxA(NULL, "Only one graphic card is supported at the moment\n", "Warning!", MB_OK);
    }

    ctx->physical_device = devices[0];

#if 0 /* it seems that Vulkan can run even if driver version doesn't match sdk version */
    VkPhysicalDeviceProperties device_props = {0};
    vkGetPhysicalDeviceProperties(ctx->physical_device, &device_props);

    if (VK_VERSION_PATCH(device_props.apiVersion) != VK_HEADER_VERSION) {
#define STR_(x) #x
#define STR(x) STR_(x)
        MessageBoxA(NULL, "Your gpu doesn't support Vulkan version 1.0." STR(VK_HEADER_VERSION), "Error", MB_OK | MB_ICONERROR);
#undef STR
#undef STR_
        ExitProcess(7);
    }
#endif

    gvVkDisplayInitSuface(display, ctx, window);

    uint32_t gq = -1, cq = -1, tq = -1, pq = -1; /* graphics queue, compute queue, tranfer queue, present queue */
    
    VkQueueFamilyProperties queue_props[32];
    VkBool32 present_support;

    uint32_t queue_props_count = sizeof(queue_props) / sizeof(queue_props[0]);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_props_count, queue_props);

#define __HAS_FLAG(x, flag) ((x) & (flag) == (flag))
#define __CHANGE(what, other1, other2, other3, to) (((what) == (other1) || (what) == (other2) || (what) == (other3)) && ((to) != (other1) && (to) != (other2) && (to) != (other3))) 
     
    for (i = 0; i < queue_props_count; i++) {
        uint32_t qf = queue_props[i].queueFlags;

        if ((gq == -1 || __CHANGE(gq, cq, tq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_GRAPHICS_BIT))
            gq = i;

        if ((cq == -1 || __CHANGE(cq, gq, tq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_COMPUTE_BIT))
            cq = i;
        
        if ((tq == -1 || __CHANGE(tq, gq, cq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_COMPUTE_BIT))
            tq = i;

        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physical_device, i, display->surface, &present_support));
        if ((pq == -1 || __CHANGE(pq, gq, cq, tq, i)) && present_support)
            pq  = i;
    }

#undef __CHANGE
#undef __HAS_FLAG

    VkDeviceQueueCreateInfo queue_cis[4] = { {0}, {0}, {0}, {0}, };
    uint32_t queue_count = 1;

    float queue_priorities[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    queue_cis[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_cis[0].queueFamilyIndex = gq;
    queue_cis[0].queueCount = 1;
    queue_cis[0].pQueuePriorities = queue_priorities;

    if (cq != gq) {
        queue_cis[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[1].queueFamilyIndex = cq;
        queue_cis[1].queueCount = 1;
        queue_cis[1].pQueuePriorities = queue_priorities;
        queue_count++;
    } else {
        queue_cis[0].queueCount = 2;
    }

    if (tq != gq && tq != cq) {
        queue_cis[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[queue_count].queueFamilyIndex = tq;
        queue_cis[queue_count].queueCount = 1;
        queue_cis[queue_count].pQueuePriorities = queue_priorities;
        queue_count++;
    } else if (tq == gq) {
        queue_cis[0].queueCount++;
    } else if (tq == cq) {
        queue_cis[1].queueCount++;
    }

    if (pq != gq && pq != tq && pq != cq) {
        queue_cis[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[queue_count].queueFamilyIndex = pq;
        queue_cis[queue_count].queueCount = 1;
        queue_cis[queue_count].pQueuePriorities = queue_priorities;
        queue_count++;
    } else if (pq == gq) {
        queue_cis[0].queueCount++;
    } else if (pq == cq) {
        queue_cis[1].queueCount++;
    } else if (pq == tq) {
        queue_cis[2].queueCount++;
    }

    VkDeviceCreateInfo device_ci = {0};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.enabledLayerCount = 0;

    VK_CHECK(vkCreateDevice(ctx->physical_device, &device_ci, NULL, &ctx->device));

    vkGetDeviceQueue(ctx->device, gq, gvMinu(0, queue_props[gq].queueCount - 1), &ctx->graphics_queue);
    vkGetDeviceQueue(ctx->device, cq, gvMinu(1, queue_props[cq].queueCount - 1), &ctx->compute_queue);
    vkGetDeviceQueue(ctx->device, tq, gvMinu(2, queue_props[tq].queueCount - 1), &ctx->transfer_queue);
    vkGetDeviceQueue(ctx->device, pq, gvMinu(3, queue_props[pq].queueCount - 1), &ctx->present_queue);

    ctx->graphics_family = gq;
    ctx->compute_family = cq;
    ctx->transfer_family = tq;
    ctx->present_family = pq;
}

void gvVkContextDestroy(struct gvVkContext *ctx) {
    vkDestroyDevice(ctx->device, NULL);
#ifdef GV_DEBUG
    fpDestroyDebugReportCallback(ctx->instance, ctx->debug_cb, NULL);
#endif
    vkDestroyInstance(ctx->instance, NULL);
}


void gvVkDisplayInit(struct gvVkDisplay *display, struct gvVkContext *ctx, struct gvWindow *window) {
    display->ctx = ctx;
    display->window = window;

    VkSurfaceFormatKHR surface_formats[8];
    uint32_t format_count = sizeof(surface_formats) / sizeof(surface_formats[0]);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, display->surface, &format_count, surface_formats);

    VkFormat color_format;
    VkColorSpaceKHR color_space;

    if (format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        color_format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        if (format_count == 0) {
            MessageBoxA(window->hwnd, "vkGetPhysicalDeviceSurfaceFormatsKHR(): no formats", "Error!", MB_OK | MB_ICONERROR);
            ExitProcess(8);
        }
        color_format =  surface_formats[0].format;
    }
    color_space =  surface_formats[0].colorSpace;

    VkCommandPoolCreateInfo pool_ci = {0};
    pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_ci.queueFamilyIndex = ctx->present_family;
    pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(ctx->device, &pool_ci, NULL, &display->cmd_pool));

    VkCommandBufferAllocateInfo cmd_buff_ai = {0};
    cmd_buff_ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buff_ai.commandBufferCount = 1;
    cmd_buff_ai.commandPool = display->cmd_pool;
    cmd_buff_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    vkAllocateCommandBuffers(ctx->device, &cmd_buff_ai, &display->cmd_buff);

    VkSwapchainCreateInfoKHR swapchain_ci = {0};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = display->surface;
    swapchain_ci.pQueueFamilyIndices = (uint32_t[]) { ctx->present_family };
    swapchain_ci.queueFamilyIndexCount = 1;
}

void gvVkDisplayDestroy(struct gvVkDisplay *display) {
    vkDestroyCommandPool(display->ctx->device, display->cmd_pool, NULL);
    vkDestroySurfaceKHR(display->ctx->instance, display->surface, NULL);
}

int WinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, char *cmd_line, int cmd_show) {
    struct gvWindow window = {0};
    struct gvVkContext ctx = {0};
    struct gvVkDisplay display = {0};

    gvWindowInit(&window, hinstance, cmd_show);
    gvVkContextInit(&ctx, &display, &window);
    gvVkDisplayInit(&display, &ctx, &window);
       
    gvWidnowLoop(&window);

    gvVkDisplayDestroy(&display);
    gvVkContextDestroy(&ctx);
    gvWindowDestroy(&window);

    ExitProcess(0);
    return 0;
}

