
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

enum gvVkQueueTypes {
    gvVkQueue_Graphics = 0,
    gvVkQueue_Compute,
    gvVkQueue_Transfer,
    gvVkQueue_Max,
};

struct gvVkContext {
    VkInstance instance;
    VkDevice device;
    VkPhysicalDevice physical_device;

#ifdef GV_DEBUG
    VkDebugReportCallbackEXT debug_cb;
#endif

    VkQueue graphics_queue;
    uint32_t graphics_index;

    VkQueue compute_queue;
    uint32_t compute_index;

    VkQueue transfer_queue;
    uint32_t transfer_index;
};

#ifdef GV_DEBUG
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
#endif

static VkBool32 VKAPI_PTR debugCallback(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT type, uint64_t obj, size_t loc, int32_t code, const char* prefix, const char* msg, void* ud) {
    OutputDebugStringA("[");
    OutputDebugStringA(prefix);
    OutputDebugStringA("] ");
    OutputDebugStringA(msg);
    OutputDebugStringA("\n");

    return VK_SUCCESS;
}

void gvVkContextInit(struct gvVkContext *ctx) {
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

    int i;

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
    vkCreateDebugReportCallback = (PFN_vkCreateDebugReportCallbackEXT) vkGetInstanceProcAddr(ctx->instance, "vkCreateDebugReportCallbackEXT");
    vkDestroyDebugReportCallback = (PFN_vkDestroyDebugReportCallbackEXT) vkGetInstanceProcAddr(ctx->instance, "vkDestroyDebugReportCallbackEXT");

    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {0};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    debug_report_ci.pfnCallback = &debugCallback;

    VK_CHECK(vkCreateDebugReportCallback(ctx->instance, &debug_report_ci, NULL, &ctx->debug_cb));
#endif

    VkPhysicalDevice devices[16];
    uint32_t devices_count = sizeof(devices) / sizeof(devices[0]);
    vkEnumeratePhysicalDevices(ctx->instance, &devices_count, devices);

    if (devices_count <= 0) {
        MessageBoxA(NULL, "Your computer doesn't support Vulkan.", "Error!", MB_OK);
        vkDestroyInstance(ctx->instance, NULL);
        ExitProcess(2);
        return 2;
    } else if (devices_count > 1) {
        MessageBoxA(NULL, "Only one graphic card is supported at the moment\n", "Warning!", MB_OK);
    }

    ctx->physical_device = devices[0];

    VkPhysicalDeviceProperties device_props = {0};
    vkGetPhysicalDeviceProperties(ctx->physical_device, &device_props);

    /* it seems that Vulkan can run even if driver version doesn't match sdk version */
    /*
    if (VK_VERSION_PATCH(device_props.apiVersion) != VK_HEADER_VERSION) {
#define STR_(x) #x
#define STR(x) STR_(x)
        MessageBoxA(NULL, "Your gpu doesn't support Vulkan version 1.0." STR(VK_HEADER_VERSION), "Error", MB_OK | MB_ICONERROR);
#undef STR
#undef STR_
        ExitProcess(7);
    }
    */
    uint32_t gq = -1, cq = -1, tq = -1; /* graphics queue, compute queue, tranfer queue */
    
    VkQueueFamilyProperties queue_props[32];
    uint32_t queue_props_count = sizeof(queue_props) / sizeof(queue_props[0]);
    vkGetPhysicalDeviceQueueFamilyProperties(ctx->physical_device, &queue_props_count, queue_props);

#define __HAS_FLAG(x, flag) ((x) & (flag) == (flag))
#define __CHANGE(what, other1, other2, to) (((what) == (other1) || (what) == (other2)) && ((to) != (other1) && (to) != (other2))) 
     
    for (i = 0; i < queue_props_count; i++) {
        uint32_t qf = queue_props[i].queueFlags;

        if ((gq == -1 || __CHANGE(gq, cq, tq, i)) && __HAS_FLAG(qf, VK_QUEUE_GRAPHICS_BIT))
            gq = i;

        if ((cq == -1 || __CHANGE(cq, gq, tq, i)) && __HAS_FLAG(qf, VK_QUEUE_COMPUTE_BIT))
            cq = i;

        if ((tq == -1 || __CHANGE(tq, gq, cq, i)) && __HAS_FLAG(qf, VK_QUEUE_TRANSFER_BIT))
            tq  = i;
    }

#undef __CHANGE
#undef __HAS_FLAG

    ctx->graphics_index = gq;
    ctx->compute_index = cq;
    ctx->transfer_index = tq;

    VkDeviceQueueCreateInfo queue_cis[3] = { {0}, {0}, {0}, };
    uint32_t queue_count = 1;

    queue_cis[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_cis[0].queueFamilyIndex = gq;
    queue_cis[0].queueCount = 1;
    queue_cis[0].pQueuePriorities = (float[]) { 1.0f };

    if (cq != gq) {
        queue_cis[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[1].queueFamilyIndex = cq;
        queue_cis[1].queueCount = 1;
        queue_cis[1].pQueuePriorities = (float[]) { 1.0f };
        queue_count++;
    }

    if (tq != gq && tq != gq) {
        queue_cis[queue_count].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_cis[queue_count].queueFamilyIndex = tq;
        queue_cis[queue_count].queueCount = 1;
        queue_cis[queue_count].pQueuePriorities = (float[]) { 1.0f };
        queue_count++;
    }

    VkDeviceCreateInfo device_ci = {0};
    device_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_ci.queueCreateInfoCount = 1;
    device_ci.pQueueCreateInfos = queue_cis;
    device_ci.enabledLayerCount = 0;

    VK_CHECK(vkCreateDevice(ctx->physical_device, &device_ci, NULL, &ctx->device));
}

void gvVkContextDestroy(struct gvVkContext *ctx) {
    vkDestroyDevice(ctx->device, NULL);
#ifdef GV_DEBUG
    vkDestroyDebugReportCallback(ctx->instance, ctx->debug_cb, NULL);
#endif
    vkDestroyInstance(ctx->instance, NULL);
}

int WinMain(HINSTANCE hinstance, HINSTANCE hprev_instance, char *cmd_line, int cmd_show) {
    struct gvVkContext ctx = {0};

    gvVkContextInit(&ctx);
    gvVkContextDestroy(&ctx);

    ExitProcess(0);
    return 0;
}

