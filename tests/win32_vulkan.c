
#include <math.h>
#define MMX_IMPLEMENTATION
#define MMX_USE_DEGREES
#define MMX_STATIC
#include <mmx/vec.h>

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


typedef struct Vertex {
    float pos[4]; 
    float col[4]; 
} Vertex;

static const Vertex vertecies[] = {
    {  0.5f,  0.5f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f, 1.0f, },
    { -0.5f,  0.5f, 0.0f, 1.0f,   0.0f, 1.0f, 0.0f, 1.0f, },
    { -0.5f, -0.5f, 0.0f, 1.0f,   1.0f, 0.0f, 0.0f, 1.0f, },
    {  0.5f, -0.5f, 0.0f, 1.0f,   1.0f, 1.0f, 1.0f, 0.0f, },
};

static const uint16_t indices[] = {
    0, 1, 2,
    0, 2, 3,
};

typedef struct GvWindow {
    HINSTANCE hinstance;
    HWND hwnd;
    int should_close : 1;
    int visible : 1;
    int width;
    int height;
    HDC hdc;

    struct GvVkLayer *curr_layer;
} GvWindow;

typedef struct GvVkContext {
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

    VkPhysicalDeviceMemoryProperties mem_props;
} GvVkContext;

typedef struct GvVkDisplay {
    struct GvVkContext *ctx;
    struct GvWindow *window;
    VkSurfaceKHR surface;
  
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buff;
    
    uint32_t curr_img;

    VkSwapchainKHR swapchain;
    uint32_t swapchain_image_count;
    VkImage swapchain_images[8];
    VkImageView swapchain_image_views[8];
    VkFormat color_format;

    VkFormat depth_format;
    VkImage depth_img;
    VkDeviceMemory depth_img_mem;
    VkImageView depth_img_view;

    VkRenderPass render_pass;
    VkFramebuffer framebuffers[8];

    VkSemaphore img_acquired;
} GvVkDisplay;

typedef struct GvVkLayer {
    struct GvVkContext *ctx;
    struct GvVkDisplay *display;

    VkBuffer ubuff;
    VkDeviceMemory ubuff_mem;

    VkDescriptorSetLayout desc_layout;
    VkPipelineLayout pipeline_layout;
    VkPipeline pipeline;
    VkDescriptorPool desc_pool;
    VkDescriptorSet desc_set;

    VkShaderModule vshader;
    VkShaderModule fshader;

    VkBuffer vbuff;
    VkDeviceMemory vbuff_mem;

    VkBuffer ibuff;
    VkDeviceMemory ibuff_mem;

    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_buff;
    VkFence fence;
} GvVkLayer;

GV_API void gvVkLayerRender(GvVkLayer *layer, GvWindow *window);

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    static PAINTSTRUCT ps;
    GvWindow *window = (GvWindow *) GetWindowLongPtr(hwnd, GWLP_USERDATA);

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
        switch (wparam) {
            case VK_ESCAPE:
                window->should_close = 1;
                return 0;
        }
        break;

    case WM_PAINT:
        if (window->curr_layer) {
            gvVkLayerRender(window->curr_layer, window);
        }
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

GV_API void gvWindowInit(GvWindow *window, HINSTANCE hinstance, int cmd_show) {
    static const char *class_name = "Some window class";
    
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = &WndProc;
    wc.hInstance = hinstance;
    wc.lpszClassName = class_name;

    RegisterClassA(&wc);

    window->width = 1280;
    window->height = 720;

    window->hwnd = CreateWindowExA(0,
                               class_name,
                               "Some window",
                               WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
                               CW_USEDEFAULT, CW_USEDEFAULT, window->width, window->height,
                               NULL, NULL,
                               hinstance,
                               NULL);

    if (window->hwnd == NULL) {
        MessageBoxA(NULL, "CreateWindowExA() failed", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(1);
    }

    RECT rect;
    GetClientRect(window->hwnd, &rect);

    window->width = rect.right - rect.left;
    window->height = rect.bottom - rect.top;


    window->hdc = GetDC(window->hwnd);

    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pf = ChoosePixelFormat(window->hdc, &pfd);
    SetPixelFormat(window->hdc, pf, &pfd);
    DescribePixelFormat(window->hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

    ShowWindow(window->hwnd, cmd_show);
    UpdateWindow(window->hwnd);

    SetWindowLongPtrA(window->hwnd, GWLP_USERDATA, (LONG_PTR) window);
}

GV_API void gvWidnowLoop(GvWindow *window) {
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

        PostMessageA(window->hwnd, WM_PAINT, 0, 0);
        SwapBuffers(window->hdc);
    }
}

GV_API void gvWindowDestroy(GvWindow *window) {
    DestroyWindow(window->hwnd);
}

GV_API void gvVkDisplayInitSuface(GvVkDisplay *display, GvVkContext *ctx, GvWindow *window) {
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

GV_API void gvVkContextInit(GvVkContext *ctx, GvVkDisplay *display, GvWindow *window) {
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

    vkGetPhysicalDeviceMemoryProperties(ctx->physical_device, &ctx->mem_props);

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

#define __HAS_FLAG(x, flag) (((x) & (flag)) == (flag))
#define __CHANGE(what, other1, other2, other3, to) (((what) == (other1) || (what) == (other2) || (what) == (other3)) && ((to) != (other1) && (to) != (other2) && (to) != (other3))) 
     
    for (i = 0; i < queue_props_count; i++) {
        uint32_t qf = queue_props[i].queueFlags;

        if ((gq == -1 || __CHANGE(gq, cq, tq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_GRAPHICS_BIT))
            gq = i;

        if ((cq == -1 || __CHANGE(cq, gq, tq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_COMPUTE_BIT))
            cq = i;
        
        if ((tq == -1 || __CHANGE(tq, gq, cq, pq, i)) && __HAS_FLAG(qf, VK_QUEUE_TRANSFER_BIT))
            tq = i;

        VK_CHECK(vkGetPhysicalDeviceSurfaceSupportKHR(ctx->physical_device, i, display->surface, &present_support));
        if ((pq == -1 || __CHANGE(pq, gq, cq, tq, i)) && present_support)
            pq  = i;
    }

#undef __CHANGE
#undef __HAS_FLAG

    /* I fucking love hard coding */

    VkDeviceQueueCreateInfo queue_cis[4] = { {0}, {0}, {0}, {0}, };
    uint32_t queue_count = 1;

    float queue_priorities[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    uint32_t gq_idx = 0;
    uint32_t cq_idx = 0;
    uint32_t tq_idx = 0;
    uint32_t pq_idx = 0;

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
        cq_idx = 1;
        queue_cis[0].queueCount = 2;
    }

    if (tq != gq && tq != cq) {
        queue_cis[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[queue_count].queueFamilyIndex = tq;
        queue_cis[queue_count].queueCount = 1;
        queue_cis[queue_count].pQueuePriorities = queue_priorities;
        queue_count++;
    } else if (tq == gq) {
        if (tq == cq) {
            tq_idx = cq_idx + 1;
        } else {
            tq_idx = 1;
        }
        queue_cis[0].queueCount++;
    } else if (tq == cq) {
        tq_idx = 1;
        queue_cis[1].queueCount++;
    }

    if (pq != gq && pq != tq && pq != cq) {
        queue_cis[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[queue_count].queueFamilyIndex = pq;
        queue_cis[queue_count].queueCount = 1;
        queue_cis[queue_count].pQueuePriorities = queue_priorities;
        queue_count++;
    } else if (pq == gq) {
        if (pq == tq) {
            pq_idx = tq_idx + 1;
        } else if (pq == cq) {
            pq_idx = cq_idx + 1;
        } else {
            pq_idx = 1;
        }
        queue_cis[0].queueCount++;
    } else if (pq == cq) {
        if (pq == tq) {
            pq_idx = tq_idx + 1;
        } else {
            pq_idx = 1;
        }
        queue_cis[1].queueCount++;
    } else if (pq == tq) {
        queue_cis[2].queueCount++;
        pq_idx = 1;
    }

    VkDeviceCreateInfo device_ci = {0};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = queue_count;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.enabledExtensionCount = 1;
    device_ci.ppEnabledExtensionNames = (const char *[]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

    VK_CHECK(vkCreateDevice(ctx->physical_device, &device_ci, NULL, &ctx->device));
    
    if (gq != -1) vkGetDeviceQueue(ctx->device, gq, gvMinu(gq_idx, queue_props[gq].queueCount - 1), &ctx->graphics_queue);
    if (cq != -1) vkGetDeviceQueue(ctx->device, cq, gvMinu(cq_idx, queue_props[cq].queueCount - 1), &ctx->compute_queue);
    if (tq != -1) vkGetDeviceQueue(ctx->device, tq, gvMinu(tq_idx, queue_props[tq].queueCount - 1), &ctx->transfer_queue);
    if (pq != -1) vkGetDeviceQueue(ctx->device, pq, gvMinu(pq_idx, queue_props[pq].queueCount - 1), &ctx->present_queue);

    ctx->graphics_family = gq;
    ctx->compute_family = cq;
    ctx->transfer_family = tq;
    ctx->present_family = pq;
}

GV_API void gvVkContextDestroy(GvVkContext *ctx) {
    vkDestroyDevice(ctx->device, NULL);
#ifdef GV_DEBUG
    fpDestroyDebugReportCallback(ctx->instance, ctx->debug_cb, NULL);
#endif
    vkDestroyInstance(ctx->instance, NULL);
}

static int pickMemoryType(GvVkContext *ctx, uint32_t typeBits, VkFlags requirements, uint32_t *typeIndex) {
    uint32_t i;
    for (i = 0; i < ctx->mem_props.memoryTypeCount; i++) {
        if ((typeBits & 1) == 1) {
            if ((ctx->mem_props.memoryTypes[i].propertyFlags & requirements) == requirements) {
                *typeIndex = i;
                return 1;
            }
        }
        typeBits >>= 1;
    }
    return 0;
}

GV_API void gvVkDisplayInit(GvVkDisplay *display, GvVkContext *ctx, GvWindow *window) {
    display->ctx = ctx;
    display->window = window;

    VkSurfaceFormatKHR surface_formats[8];
    uint32_t format_count = sizeof(surface_formats) / sizeof(surface_formats[0]);
    vkGetPhysicalDeviceSurfaceFormatsKHR(ctx->physical_device, display->surface, &format_count, surface_formats);

    VkColorSpaceKHR color_space;

    if (format_count == 1 && surface_formats[0].format == VK_FORMAT_UNDEFINED) {
        display->color_format = VK_FORMAT_B8G8R8A8_UNORM;
    } else {
        if (format_count == 0) {
            MessageBoxA(window->hwnd, "vkGetPhysicalDeviceSurfaceFormatsKHR(): no formats", "Error!", MB_OK | MB_ICONERROR);
            ExitProcess(8);
        }
        display->color_format =  surface_formats[0].format;
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

    
    VkSurfaceCapabilitiesKHR surface_caps = {0};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(ctx->physical_device, display->surface, &surface_caps);

    VkSwapchainCreateInfoKHR swapchain_ci = {0};
    swapchain_ci.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchain_ci.surface = display->surface;
    swapchain_ci.minImageCount = gvMinu(surface_caps.minImageCount + 1, surface_caps.maxImageCount);
    swapchain_ci.imageFormat = display->color_format;
    swapchain_ci.imageExtent.width = gvClamp(window->width, surface_caps.minImageExtent.width, surface_caps.maxImageExtent.width);
    swapchain_ci.imageExtent.height = gvClamp(window->height, surface_caps.minImageExtent.height, surface_caps.maxImageExtent.height);
    
    if ((surface_caps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) == VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR) {
        swapchain_ci.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
    } else {
        swapchain_ci.preTransform = surface_caps.currentTransform;
    }
    
    swapchain_ci.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchain_ci.imageArrayLayers = 1;
    swapchain_ci.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchain_ci.oldSwapchain = display->swapchain;
    swapchain_ci.clipped = VK_TRUE;
    swapchain_ci.imageColorSpace = color_space;
    swapchain_ci.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
 
    if (ctx->graphics_family != ctx->present_family) {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swapchain_ci.queueFamilyIndexCount = 2;
        swapchain_ci.pQueueFamilyIndices = (uint32_t[]) { ctx->graphics_family, ctx->present_family, };
    } else {
        swapchain_ci.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        swapchain_ci.queueFamilyIndexCount = 0;
        swapchain_ci.pQueueFamilyIndices = NULL;
    }

    VK_CHECK(vkCreateSwapchainKHR(ctx->device, &swapchain_ci, NULL, &display->swapchain));

    display->swapchain_image_count = sizeof(display->swapchain_images) / sizeof(display->swapchain_images[0]);
    VK_CHECK(vkGetSwapchainImagesKHR(ctx->device, display->swapchain, &display->swapchain_image_count, display->swapchain_images));

    VkImageViewCreateInfo img_view_ci = {0};
    img_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    img_view_ci.viewType = VK_IMAGE_TYPE_2D;
    img_view_ci.format = display->color_format;
    img_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    img_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    img_view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
    img_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    img_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    img_view_ci.subresourceRange.baseMipLevel = 0;
    img_view_ci.subresourceRange.levelCount = 1;
    img_view_ci.subresourceRange.baseArrayLayer = 0;
    img_view_ci.subresourceRange.layerCount = 1;

    uint32_t i;
    for (i = 0; i < display->swapchain_image_count; i++) {
        img_view_ci.image = display->swapchain_images[i];
        VK_CHECK(vkCreateImageView(ctx->device, &img_view_ci, NULL, &display->swapchain_image_views[i]));
    }

    VkImageCreateInfo depth_img_ci = {0};
    depth_img_ci.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depth_img_ci.imageType = VK_IMAGE_TYPE_2D;
    depth_img_ci.format = display->depth_format = VK_FORMAT_D16_UNORM;
    depth_img_ci.extent.width = window->width;
    depth_img_ci.extent.height = window->height;
    depth_img_ci.extent.depth = 1;
    depth_img_ci.mipLevels = 1;
    depth_img_ci.arrayLayers = 1;
    depth_img_ci.samples = VK_SAMPLE_COUNT_1_BIT;
    depth_img_ci.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depth_img_ci.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    depth_img_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateImage(ctx->device, &depth_img_ci, NULL, &display->depth_img);

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(ctx->device, display->depth_img, &mem_reqs);

    VkMemoryAllocateInfo mem_alloc = {0};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.allocationSize = mem_reqs.size;

    if (!pickMemoryType(ctx, mem_reqs.memoryTypeBits, 0, &mem_alloc.memoryTypeIndex)) {
        MessageBoxA(NULL, "Unable to find suitable memory for depth image", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(12);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &mem_alloc, NULL, &display->depth_img_mem));
    VK_CHECK(vkBindImageMemory(ctx->device, display->depth_img, display->depth_img_mem, 0));

    VkImageViewCreateInfo depth_img_view_ci = {0};
    depth_img_view_ci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    depth_img_view_ci.image = display->depth_img;
    depth_img_view_ci.format = display->depth_format = VK_FORMAT_D16_UNORM;
    depth_img_view_ci.components.r = VK_COMPONENT_SWIZZLE_R;
    depth_img_view_ci.components.g = VK_COMPONENT_SWIZZLE_G;
    depth_img_view_ci.components.b = VK_COMPONENT_SWIZZLE_B;
    depth_img_view_ci.components.a = VK_COMPONENT_SWIZZLE_A;
    depth_img_view_ci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depth_img_view_ci.subresourceRange.baseMipLevel = 0;
    depth_img_view_ci.subresourceRange.levelCount = 1;
    depth_img_view_ci.subresourceRange.baseArrayLayer = 0;
    depth_img_view_ci.subresourceRange.layerCount = 1;
    depth_img_view_ci.viewType = VK_IMAGE_VIEW_TYPE_2D;

    VK_CHECK(vkCreateImageView(ctx->device, &depth_img_view_ci, NULL, &display->depth_img_view));


    VkAttachmentDescription attachments[2] = { {0}, {0} };
    
    attachments[0].format = display->color_format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachments[0].flags = 0;

    attachments[1].format = display->depth_format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachments[1].flags = 0;

    VkAttachmentReference color_reference = {0};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depth_reference = {0};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {0};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &color_reference;
    subpass.pDepthStencilAttachment = &depth_reference;

    VkRenderPassCreateInfo render_pass_ci = {0};
    render_pass_ci.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    render_pass_ci.attachmentCount = 2;
    render_pass_ci.pAttachments = attachments;
    render_pass_ci.subpassCount = 1;
    render_pass_ci.pSubpasses = &subpass;   

    VK_CHECK(vkCreateRenderPass(ctx->device, &render_pass_ci, NULL, &display->render_pass));

    VkImageView framebuffer_attachments[2];
    framebuffer_attachments[1] = display->depth_img_view;

    VkFramebufferCreateInfo framebuffer_ci = {0};
    framebuffer_ci.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebuffer_ci.width = window->width;
    framebuffer_ci.height = window->height;
    framebuffer_ci.renderPass = display->render_pass;
    framebuffer_ci.attachmentCount = 2;
    framebuffer_ci.pAttachments = framebuffer_attachments;
    framebuffer_ci.layers = 1;

    for (i = 0; i < display->swapchain_image_count; i++) {
        framebuffer_attachments[0] = display->swapchain_image_views[i];
        VK_CHECK(vkCreateFramebuffer(ctx->device, &framebuffer_ci, NULL, &display->framebuffers[i]));
    }

    VkSemaphoreCreateInfo semaphore_ci = {0};
    semaphore_ci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VK_CHECK(vkCreateSemaphore(ctx->device, &semaphore_ci, NULL, &display->img_acquired));

}

GV_API void gvVkDisplaySelect(GvVkDisplay *display) {
    VK_CHECK(vkAcquireNextImageKHR(display->ctx->device, display->swapchain, UINT64_MAX, display->img_acquired, NULL, &display->curr_img));
}

GV_API void gvVkDisplayPresent(GvVkDisplay *display) {
    VkPresentInfoKHR present = {0};
    present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present.swapchainCount = 1;
    present.pImageIndices = &display->curr_img;
    present.pSwapchains = &display->swapchain;
    
    VK_CHECK(vkQueuePresentKHR(display->ctx->present_queue, &present));
}

GV_API void gvVkDisplayDestroy(GvVkDisplay *display) {
    uint32_t i;

    vkDestroySemaphore(display->ctx->device, display->img_acquired, NULL);

    for (i = 0; i < display->swapchain_image_count; i++) {
        vkDestroyFramebuffer(display->ctx->device, display->framebuffers[i], NULL);
    }
    vkDestroyRenderPass(display->ctx->device, display->render_pass, NULL);

    vkDestroyImageView(display->ctx->device, display->depth_img_view, NULL);
    vkFreeMemory(display->ctx->device, display->depth_img_mem, NULL);
    vkDestroyImage(display->ctx->device, display->depth_img, NULL);

    for (i = 0; i < display->swapchain_image_count; i++) {
        vkDestroyImageView(display->ctx->device, display->swapchain_image_views[i], NULL);
    }

    vkDestroySwapchainKHR(display->ctx->device, display->swapchain, NULL);
    vkDestroyCommandPool(display->ctx->device, display->cmd_pool, NULL);
    vkDestroySurfaceKHR(display->ctx->instance, display->surface, NULL);
}

GV_API void gvVkLayerInit(GvVkContext *ctx, GvVkDisplay *display, GvVkLayer *layer) {
    layer->ctx = ctx;
    layer->display = display;

    VkBufferCreateInfo ubuff_ci = {0};
    ubuff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ubuff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    ubuff_ci.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    ubuff_ci.size = sizeof(float[16]);

    VK_CHECK(vkCreateBuffer(ctx->device, &ubuff_ci, NULL, &layer->ubuff));

    VkMemoryRequirements mem_reqs = {0};
    vkGetBufferMemoryRequirements(ctx->device, layer->ubuff, &mem_reqs);

    VkMemoryAllocateInfo ubuff_ai = {0};
    ubuff_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ubuff_ai.allocationSize = mem_reqs.size;

    if (!pickMemoryType(ctx, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ubuff_ai.memoryTypeIndex)) {
        MessageBoxA(NULL, "Unable to find suitable memory for uniform buffer", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(12);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &ubuff_ai, NULL, &layer->ubuff_mem));

    void *dev_mem;
    VK_CHECK(vkMapMemory(ctx->device, layer->ubuff_mem, 0, sizeof(float[16]), 0, &dev_mem));

    xm4_ortho((float *) dev_mem, -3, 3, -3, 3);

    vkUnmapMemory(ctx->device, layer->ubuff_mem);

    VK_CHECK(vkBindBufferMemory(ctx->device, layer->ubuff, layer->ubuff_mem, 0));

    VkDescriptorSetLayoutBinding layout_binding = {0};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    VkDescriptorSetLayoutCreateInfo desc_layout_ci = {0};
    desc_layout_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    desc_layout_ci.bindingCount = 1;
    desc_layout_ci.pBindings = &layout_binding;

    VK_CHECK(vkCreateDescriptorSetLayout(ctx->device, &desc_layout_ci, NULL, &layer->desc_layout));

    VkPipelineLayoutCreateInfo pipeline_layout_ci = {0};
    pipeline_layout_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_ci.setLayoutCount = 1;
    pipeline_layout_ci.pSetLayouts = &layer->desc_layout;

    VK_CHECK(vkCreatePipelineLayout(ctx->device, &pipeline_layout_ci, NULL, &layer->pipeline_layout));
    
    VkDescriptorPoolSize type_count[1];
    type_count[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    type_count[0].descriptorCount = 1;
    
    VkDescriptorPoolCreateInfo desc_pool_ci = {0};
    desc_pool_ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    desc_pool_ci.maxSets = 1;
    desc_pool_ci.poolSizeCount = 1;
    desc_pool_ci.pPoolSizes = type_count;

    VK_CHECK(vkCreateDescriptorPool(ctx->device, &desc_pool_ci, NULL, &layer->desc_pool));

    VkDescriptorSetAllocateInfo desc_set_ai = {0};
    desc_set_ai.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    desc_set_ai.descriptorPool = layer->desc_pool;
    desc_set_ai.descriptorSetCount = 1;
    desc_set_ai.pSetLayouts = &layer->desc_layout;
    
    VK_CHECK(vkAllocateDescriptorSets(ctx->device, &desc_set_ai, &layer->desc_set));

    VkDescriptorBufferInfo ubuff_info = {0};
    ubuff_info.buffer = layer->ubuff;
    ubuff_info.offset = 0;
    ubuff_info.range = sizeof(float[16]);

    VkWriteDescriptorSet write_desc_set = {0};
    write_desc_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write_desc_set.dstSet = layer->desc_set;
    write_desc_set.descriptorCount = 1;
    write_desc_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    write_desc_set.pBufferInfo = &ubuff_info;
    write_desc_set.dstArrayElement = 0;
    write_desc_set.dstBinding = 0;

    vkUpdateDescriptorSets(ctx->device, 1, &write_desc_set, 0, NULL);

    size_t vshader_len;
    const char *vshader_code = stb_filec("vert.spv", &vshader_len);

    if (!vshader_code) {
        MessageBoxA(NULL, "Unable to load 'vert.spv'", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(14);
    }

    VkShaderModuleCreateInfo vshader_ci = {0};
    vshader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vshader_ci.codeSize = vshader_len;
    vshader_ci.pCode = vshader_code;
    
    VK_CHECK(vkCreateShaderModule(ctx->device, &vshader_ci, NULL, &layer->vshader));

    VkPipelineShaderStageCreateInfo shader_stages_ci[2] = { {0}, {0} };

    shader_stages_ci[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages_ci[0].pName = "main";
    shader_stages_ci[0].module = layer->vshader;
    shader_stages_ci[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    
    size_t fshader_len;
    const char *fshader_code = stb_filec("frag.spv", &fshader_len);

    if (!fshader_code) {
        MessageBoxA(NULL, "Unable to load 'frag.spv'", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(14);
    }

    VkShaderModuleCreateInfo fshader_ci = {0};
    fshader_ci.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fshader_ci.codeSize = fshader_len;
    fshader_ci.pCode = fshader_code;
    
    VK_CHECK(vkCreateShaderModule(ctx->device, &fshader_ci, NULL, &layer->fshader));

    shader_stages_ci[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shader_stages_ci[1].pName = "main";
    shader_stages_ci[1].module = layer->fshader;
    shader_stages_ci[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    

    VkBufferCreateInfo vbuff_ci = {0};
    vbuff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    vbuff_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbuff_ci.size = sizeof(vertecies);
    vbuff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(ctx->device, &vbuff_ci, NULL, &layer->vbuff));

    vkGetBufferMemoryRequirements(ctx->device, layer->vbuff, &mem_reqs);

    VkMemoryAllocateInfo vbuff_ai = {0};
    vbuff_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    vbuff_ai.allocationSize = mem_reqs.size;
    
    if (!pickMemoryType(ctx, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &vbuff_ai.memoryTypeIndex)) {
        MessageBoxA(NULL, "Unable to find suitable memory for vertex buffer", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(12);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &vbuff_ai, NULL, &layer->vbuff_mem));

    void *vbuff_data;
    VK_CHECK(vkMapMemory(ctx->device, layer->vbuff_mem, 0, mem_reqs.size, 0, &vbuff_data));
    memcpy(vbuff_data, vertecies, sizeof(vertecies));
    vkUnmapMemory(ctx->device, layer->vbuff_mem);

    vkBindBufferMemory(ctx->device, layer->vbuff, layer->vbuff_mem, 0);

    VkBufferCreateInfo ibuff_ci = {0};
    ibuff_ci.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    ibuff_ci.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    ibuff_ci.size = sizeof(indices);
    ibuff_ci.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VK_CHECK(vkCreateBuffer(ctx->device, &ibuff_ci, NULL, &layer->ibuff));

    vkGetBufferMemoryRequirements(ctx->device, layer->ibuff, &mem_reqs);

    VkMemoryAllocateInfo ibuff_ai = {0};
    ibuff_ai.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    ibuff_ai.allocationSize = mem_reqs.size;
    
    if (!pickMemoryType(ctx, mem_reqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, &ibuff_ai.memoryTypeIndex)) {
        MessageBoxA(NULL, "Unable to find suitable memory for vertex buffer", "Error!", MB_OK | MB_ICONERROR);
        ExitProcess(12);
    }

    VK_CHECK(vkAllocateMemory(ctx->device, &ibuff_ai, NULL, &layer->ibuff_mem));

    void *ibuff_data;
    VK_CHECK(vkMapMemory(ctx->device, layer->ibuff_mem, 0, mem_reqs.size, 0, &ibuff_data));
    memcpy(ibuff_data, indices, sizeof(indices));
    vkUnmapMemory(ctx->device, layer->ibuff_mem);

    vkBindBufferMemory(ctx->device, layer->ibuff, layer->ibuff_mem, 0);

    VkCommandPoolCreateInfo cmd_pool_ci = {0};
    cmd_pool_ci.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_ci.queueFamilyIndex = ctx->graphics_family;
    cmd_pool_ci.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    VK_CHECK(vkCreateCommandPool(ctx->device, &cmd_pool_ci, NULL, &layer->cmd_pool));

    VkCommandBufferAllocateInfo cmd_buff_ai = {0};
    cmd_buff_ai.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmd_buff_ai.commandBufferCount = 1;
    cmd_buff_ai.commandPool = layer->cmd_pool;
    cmd_buff_ai.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    VK_CHECK(vkAllocateCommandBuffers(ctx->device, &cmd_buff_ai, &layer->cmd_buff));

    VkDynamicState dyn_states[VK_DYNAMIC_STATE_RANGE_SIZE] = {0};
    VkPipelineDynamicStateCreateInfo dyn_state_ci = {0};
    dyn_state_ci.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dyn_state_ci.pDynamicStates = dyn_states;
    dyn_state_ci.dynamicStateCount = 0;

    VkVertexInputBindingDescription vbinding = {0};
    vbinding.binding = 0;
    vbinding.binding = VK_VERTEX_INPUT_RATE_VERTEX;
    vbinding.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription vattrs[2] = { {0}, {0} };
    vattrs[0].binding = 0;
    vattrs[0].location = 0;
    vattrs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vattrs[0].offset = &((Vertex *) NULL)->pos[0];

    vattrs[1].binding = 0;
    vattrs[1].location = 1;
    vattrs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    vattrs[1].offset = &((Vertex *) NULL)->col[0];

    VkPipelineVertexInputStateCreateInfo vi = {0};
    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = &vbinding;
    vi.vertexAttributeDescriptionCount = 2;
    vi.pVertexAttributeDescriptions = vattrs;

    VkPipelineInputAssemblyStateCreateInfo ia = {0};
    ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    ia.primitiveRestartEnable = VK_FALSE;
    ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    VkPipelineRasterizationStateCreateInfo rs = {0};
    rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rs.polygonMode = VK_POLYGON_MODE_FILL;
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
    rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rs.depthClampEnable = VK_TRUE;
    rs.rasterizerDiscardEnable = VK_FALSE;
    rs.depthBiasEnable = VK_FALSE;
    rs.depthBiasConstantFactor = 0;
    rs.depthBiasClamp = 0;
    rs.depthBiasSlopeFactor = 0;
    rs.lineWidth = 1.0f;

    VkPipelineColorBlendAttachmentState att_state[1] = { {0} };
    att_state[0].colorWriteMask = 0xf;
    att_state[0].blendEnable = VK_FALSE;
    att_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
    att_state[0].colorBlendOp = VK_BLEND_OP_ADD;
    att_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    att_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;

    VkPipelineColorBlendStateCreateInfo cb = {0};
    cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    cb.attachmentCount = 1;
    cb.pAttachments = att_state;
    cb.logicOpEnable = VK_FALSE;
    cb.logicOp = VK_LOGIC_OP_NO_OP;
    cb.blendConstants[0] = 1.0f;
    cb.blendConstants[1] = 1.0f;
    cb.blendConstants[2] = 1.0f;
    cb.blendConstants[3] = 1.0f;

    VkPipelineViewportStateCreateInfo vp = {0};
    vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    vp.pNext = NULL;
    vp.flags = 0;
    vp.viewportCount = 1;
    vp.scissorCount = 1;
    vp.pScissors = NULL;
    vp.pViewports = NULL;
    dyn_states[dyn_state_ci.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
    dyn_states[dyn_state_ci.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

    VkPipelineDepthStencilStateCreateInfo ds = {0};
    ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    ds.pNext = NULL;
    ds.flags = 0;
    ds.depthTestEnable = VK_TRUE;
    ds.depthWriteEnable = VK_TRUE;
    ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    ds.depthBoundsTestEnable = VK_FALSE;
    ds.minDepthBounds = 0;
    ds.maxDepthBounds = 0;
    ds.stencilTestEnable = VK_FALSE;
    ds.back.failOp = VK_STENCIL_OP_KEEP;
    ds.back.passOp = VK_STENCIL_OP_KEEP;
    ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
    ds.back.compareMask = 0;
    ds.back.reference = 0;
    ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
    ds.back.writeMask = 0;
    ds.front = ds.back;

    VkPipelineMultisampleStateCreateInfo ms = {0};
    ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    ms.pNext = NULL;
    ms.flags = 0;
    ms.pSampleMask = NULL;
    ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    ms.sampleShadingEnable = VK_FALSE;
    ms.alphaToCoverageEnable = VK_FALSE;
    ms.alphaToOneEnable = VK_FALSE;
    ms.minSampleShading = 0.0;

    VkGraphicsPipelineCreateInfo pipeline_ci = {0};
    pipeline_ci.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline_ci.pNext = NULL;
    pipeline_ci.layout = layer->pipeline_layout;
    pipeline_ci.basePipelineHandle = NULL;
    pipeline_ci.basePipelineIndex = 0;
    pipeline_ci.flags = 0;
    pipeline_ci.pVertexInputState = &vi;
    pipeline_ci.pInputAssemblyState = &ia;
    pipeline_ci.pRasterizationState = &rs;
    pipeline_ci.pColorBlendState = &cb;
    pipeline_ci.pTessellationState = NULL;
    pipeline_ci.pMultisampleState = &ms;
    pipeline_ci.pDynamicState = &dyn_state_ci;
    pipeline_ci.pViewportState = &vp;
    pipeline_ci.pDepthStencilState = &ds;
    pipeline_ci.pStages = shader_stages_ci;
    pipeline_ci.stageCount = 2;
    pipeline_ci.renderPass = layer->display->render_pass;
    pipeline_ci.subpass = 0;

    vkCreateGraphicsPipelines(ctx->device, NULL, 1, &pipeline_ci, NULL, &layer->pipeline);

    VkFenceCreateInfo fence_ci = {0};
    fence_ci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    VK_CHECK(vkCreateFence(ctx->device, &fence_ci, NULL, &layer->fence));

}

GV_API void gvVkLayerRender(GvVkLayer *layer, GvWindow *window) {
    gvVkDisplaySelect(layer->display);
    
    VkCommandBufferBeginInfo cmd_bi = {0};
    cmd_bi.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_bi.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    VK_CHECK(vkBeginCommandBuffer(layer->cmd_buff, &cmd_bi));
    
    VkClearValue clear_values[2];
    clear_values[0].color.float32[0] = 0.0f;
    clear_values[0].color.float32[1] = 0.0f;
    clear_values[0].color.float32[2] = 0.0f;
    clear_values[0].color.float32[3] = 0.0f;
    clear_values[1].depthStencil.depth = 1.0f;
    clear_values[1].depthStencil.stencil = 0;

    VkRenderPassBeginInfo render_pass_bi = {0};
    render_pass_bi.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_bi.renderPass = layer->display->render_pass;
    render_pass_bi.framebuffer = layer->display->framebuffers[layer->display->curr_img];
    render_pass_bi.renderArea.offset.x = 0.0f;
    render_pass_bi.renderArea.offset.y = 0.0f;
    render_pass_bi.renderArea.extent.width = window->width;
    render_pass_bi.renderArea.extent.height = window->height;
    render_pass_bi.clearValueCount = 2;
    render_pass_bi.pClearValues = clear_values;

    vkCmdBeginRenderPass(layer->cmd_buff, &render_pass_bi, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(layer->cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, layer->pipeline);
    vkCmdBindDescriptorSets(layer->cmd_buff, VK_PIPELINE_BIND_POINT_GRAPHICS, layer->pipeline_layout, 0, 1, &layer->desc_set, 0, NULL);

    VkViewport viewport = {0};
    viewport.width = window->width;
    viewport.height = window->height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    viewport.x = 0;
    viewport.y = 0;    
    vkCmdSetViewport(layer->cmd_buff, 0, 1, &viewport);

    VkRect2D scissor = {0};
    scissor.offset.x = 0.0f;
    scissor.offset.y = 0.0f;
    scissor.extent.width = window->width;
    scissor.extent.height = window->height;
    vkCmdSetScissor(layer->cmd_buff, 0, 1, &scissor);

    vkCmdBindIndexBuffer(layer->cmd_buff, layer->ibuff, 0, VK_INDEX_TYPE_UINT16);
    vkCmdBindVertexBuffers(layer->cmd_buff, 0, 1, &layer->vbuff, (VkDeviceSize[]) { 0 });
    vkCmdDrawIndexed(layer->cmd_buff, sizeof(indices) / sizeof(indices[0]), 1, 0, 0, 0);
    vkCmdEndRenderPass(layer->cmd_buff);

    VK_CHECK(vkEndCommandBuffer(layer->cmd_buff));

    VkPipelineStageFlags pipe_stage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    VkSubmitInfo submit_info = {0};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &layer->cmd_buff;
    submit_info.pWaitDstStageMask = &pipe_stage;
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &layer->display->img_acquired;

    VK_CHECK(vkQueueSubmit(layer->ctx->graphics_queue, 1, &submit_info, layer->fence));

    VK_CHECK(vkWaitForFences(layer->ctx->device, 1, &layer->fence, VK_TRUE, UINT64_MAX));
    VK_CHECK(vkResetFences(layer->ctx->device, 1, &layer->fence));

    gvVkDisplayPresent(layer->display);
}

GV_API void gvVkLayerDestroy(GvVkLayer *layer) {
    vkDestroyFence(layer->ctx->device, layer->fence, NULL);

    vkDestroyPipeline(layer->ctx->device, layer->pipeline, NULL);

    vkDestroyCommandPool(layer->ctx->device, layer->cmd_pool, NULL);

    vkFreeMemory(layer->ctx->device, layer->ibuff_mem, NULL);
    vkDestroyBuffer(layer->ctx->device, layer->ibuff, NULL);
    
    vkFreeMemory(layer->ctx->device, layer->vbuff_mem, NULL);
    vkDestroyBuffer(layer->ctx->device, layer->vbuff, NULL);

    vkDestroyShaderModule(layer->ctx->device, layer->vshader, NULL);
    vkDestroyShaderModule(layer->ctx->device, layer->fshader, NULL);

    vkDestroyDescriptorPool(layer->ctx->device, layer->desc_pool, NULL);

    vkDestroyPipelineLayout(layer->ctx->device, layer->pipeline_layout, NULL);
    vkDestroyDescriptorSetLayout(layer->ctx->device, layer->desc_layout, NULL);
    
    vkFreeMemory(layer->ctx->device, layer->ubuff_mem, NULL);
    vkDestroyBuffer(layer->ctx->device, layer->ubuff, NULL);
}

int WinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, char *cmd_line, int cmd_show) {
    GvWindow window = {0};
    GvVkContext ctx = {0};
    GvVkDisplay display = {0};
    GvVkLayer layer = {0};

    gvWindowInit(&window, hinstance, cmd_show);
    gvVkContextInit(&ctx, &display, &window);
    gvVkDisplayInit(&display, &ctx, &window);
    gvVkLayerInit(&ctx, &display, &layer);

    window.curr_layer = &layer;

    gvWidnowLoop(&window);

    gvVkLayerDestroy(&layer);
    gvVkDisplayDestroy(&display);
    gvVkContextDestroy(&ctx);
    gvWindowDestroy(&window);

    ExitProcess(0);
    return 0;
}

