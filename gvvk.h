/*
    gvvk.h - private code - Bart³omiej's utilities made for vulkan

ABOUT:
    Simple library made to make using Vulkan more pleasant.

DEFINE:
    GVVK_IMPLEMENTATION - to define functions

    GVVK_STATIC - to enable static linking

    GVVK_DEBUG - to add some checks and debugging

    GVVK_USER_REQUESTED_FUNCTIONS - to include functions for user 
        that are not defined and included in function table (X macro)

    GVVK_IGNORE_GV_DEBUG - to ignore GV_DEBUG
    
    GVVK_IGNORE_GV_STATIC - to ignore GV_STATIC

    GVVK_DOWN_FORCE_INLINE - to not force inline

    GVVK_DYNAMIC_LINK_STD - when you are linking std dynamicly

    GV_DEBUG - to define GVVK_DEBUG automaticly

    GV_STATIC - to define GVVK_STATIC automaticly

NOTES:
    - I suggest to abbrivate GVVK_CHECK to VK using
        #define GVVK_CHECK VK

AUTHOR:
    Bart³omiej Grzesik

 */


#ifdef __cplusplus
extern "C" {
#endif

#ifndef __GV_VULKAN__
#define __GV_VULKAN__ 1


#if !defined(GVVK_DEBUG) && defined(GV_DEBUG) && !defined(GVVK_IGNORE_GV_DEBUG)
#define GVVK_DEBUG GV_DEBUG
#endif

#if !defined(GVVK_STATIC) && defined(GV_STATIC) && !defined(GVVK_IGNORE_GV_STATIC)
#define GVVK_STATIC
#endif

#if defined(GVVK_STATIC) && !defined(GVVK_API) 
#define GVVK_API static
#elif !defined(GVVK_API)
#define GVVK_API extern
#endif

#ifndef GVVK_INTERN
#define GVVK_INTERN static
#endif

#ifndef GVVK_FORCE_INLINE
#if defined(_MSC_VER) && !defined(GVVK_DONT_FORCE_INLINE)
#define GVVK_FORCE_INLINE static inline __forceinline
#elif !defined(GVVK_DONT_FORCE_INLINE) 
#define GVVK_FORCE_INLINE static inline __attribute__((always_inline))
#else
#define GVVK_FORCE_INLINE static inline
#endif
#endif

#ifndef GVVK__ARR_SIZE
#define GVVK__ARR_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif

#if defined(GVVK_DEBUG) && !defined(GVVK__LOG)
#ifdef _WIN32
extern __declspec(dllimport) void __stdcall OutputDebugStringA(const char *);
#define GVVK__LOG OutputDebugStringA
#else
extern int puts(const char *str);
#define GVVK__LOG puts
#endif
#elif !defined(GVVK__LOG)
#define GVVK__LOG(...)
#endif

#if defined(GVVK_DEBUG) && !defined(GVVK__ASSERT)
#include <assert.h>
#define GVVK__ASSERT(cond) assert(cond)
#elif !defined(GVVK__ASSERT)
#define GVVK__ASSERT(cond) (cond)
#endif

#ifndef GVVK__BREAK
#ifdef _WIN32
#define GVVK__BREAK() __debugbreak()
#else
#define GVVK__BREAK() asm("int $3")
#endif
#endif

#if defined(_MSC_VER) && !defined(GVVK__CONSTRUCTOR)
#ifdef _WIN64
#define GVVK__CONSTRUCTOR_PREFIX ""
#else
#define GVVK__CONSTRUCTOR_PREFIX "_"
#endif
#pragma section(".CRT$XCU", read)
#define GVVK__CONSTRUCTOR(fn)                                                 \
    static void fn(void);                                                     \
    __pragma(section(".CRT$XCU", read))                                       \
    __declspec(allocate(".CRT$XCU")) void (*fn##_f)(void) = &fn;              \
    __pragma(comment(linker, "/include:" GVVK__CONSTRUCTOR_PREFIX #fn "_f"))  \
    static void fn(void)
#elif !defined(GVVK__CONSTRUCTOR)
#define GVVK__CONSTRUCTOR(fn) static void __attribute__ ((unused, constructor)) fn(void)
#endif

#if defined(_MSC_VER) && !defined(GVVK__DESTRUCTOR)
#ifdef GVVK_DYNAMIC_LINK_STD
__declspec(dllimport) extern int atexit(void (__cdecl *func)(void));
#else
extern int atexit(void (__cdecl *func)(void));
#endif
#define GVVK__DESTRUCTOR(fn)                                            \
    static void __CRTDECL fn(void);                                     \
    GVVK__CONSTRUCTOR(fn##___constructor) { atexit(&fn); }              \
    static void __CRTDECL fn(void)
#elif !defined(GVVK__DESTRUCTOR)
#define GVVK__DESTRUCTOR(fn) static void __attribute__ ((unused, destructor)) fn(void)
#endif

#ifndef GVVK_OFFSETOF
#define GVVK_OFFSETOF(type, member) (&((type *) (void *) 0)->member)
#endif

typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

#if defined(_WIN64)
typedef uint64_t intptr_t;
#else
typedef uint32_t intptr_t;
#endif

typedef intptr_t size_t;
typedef intptr_t ptrdiff_t;

//#include <stdint.h>
#define VK_NO_PROTOTYPES
#include <vulkan/vulkan.h>

#ifdef _WIN32
//#include <Windows.h>



/* WinAPI without <windows.h> (increases performace) */
typedef struct HINSTANCE__  *HINSTANCE;
typedef struct HWND__       *HWND;
typedef struct HDC__        *HDC;
typedef HINSTANCE            HMODULE;
typedef unsigned int         UINT;
typedef unsigned int        *WPARAM;
typedef long                *LPARAM;
typedef long                *LRESULT;

typedef long *(__stdcall* WNDPROC)(HWND, UINT, WPARAM, LPARAM);
typedef struct {
    UINT                 style;
    WNDPROC              lpfnWndProc;
    int                  cbClsExtra;
    int                  cbWndExtra;
    HINSTANCE            hInstance;
    struct HICON__      *hIcon;
    struct HCURSOR__    *hCursor;
    struct HBRUSH__     *hbrBackground;
    const char          *lpszMenuName;
    const char          *lpszClassName;
} WNDCLASSA;

typedef struct {
    long left, top, right, bottom;
} RECT;

typedef struct {
    long x, y;
} POINT;

typedef struct {
    HWND            hwnd;
    UINT            message;
    WPARAM          wParam;
    LPARAM          lParam;
    unsigned long   time;
    POINT           pt;
#ifdef _MAC
    DWORD       lPrivate;
#endif
} MSG;

extern __declspec(dllimport) intptr_t (__cdecl *__stdcall GetProcAddress(HMODULE, const char *))();
extern __declspec(dllimport) HMODULE __stdcall LoadLibraryA(const char*);
extern __declspec(dllimport) int __stdcall FreeLibrary(HMODULE);

extern __declspec(dllimport) unsigned short __stdcall RegisterClassA(const WNDCLASSA *);
extern __declspec(dllimport) long *__stdcall DefWindowProcA(HWND, UINT, WPARAM, LPARAM);
extern __declspec(dllimport) long *__stdcall PostMessageA(HWND, UINT, WPARAM, LPARAM);
extern __declspec(dllimport) int __stdcall ShowWindow(HWND, int);
extern __declspec(dllimport) int __stdcall UpdateWindow(HWND);
extern __declspec(dllimport) int __stdcall DestroyWindow(HWND);
extern __declspec(dllimport) HDC __stdcall GetDC(HWND);
extern __declspec(dllimport) int __stdcall GetClientRect(HWND, RECT *);
extern __declspec(dllimport) int __stdcall PeekMessageA(MSG *, HWND, unsigned, unsigned, unsigned);
extern __declspec(dllimport) int __stdcall GetMessageA(MSG *, HWND, unsigned, unsigned);
extern __declspec(dllimport) int __stdcall TranslateMessage(const MSG *);
extern __declspec(dllimport) long *__stdcall DispatchMessageA(const MSG *);
extern __declspec(dllimport) int __stdcall SwapBuffers(HDC);
extern __declspec(dllimport) HWND __stdcall CreateWindowExA(unsigned long, const char *, const char *, unsigned long, int, int, int, int, HWND, struct HMENU__ *, HINSTANCE, void *);
extern __declspec(dllimport) void __stdcall PostQuitMessage(int);

#ifndef WS_OVERLAPPED
#define WS_OVERLAPPED       0x00000000L
#define WS_CAPTION          0x00C00000L
#define WS_THICKFRAME       0x00040000L
#define WS_MINIMIZEBOX      0x00020000L
#define WS_SYSMENU          0x00080000L
#define WS_VISIBLE          0x10000000L
#define WM_PAINT            0x000F
#define PM_NOREMOVE         0x0000
#define WM_CLOSE            0x0010
#endif

/* confuses my code */
#undef CreateSemaphore
#undef CreateEvent
#undef SetEvent
#undef ResetEvent

#pragma comment(lib, "vulkan-1.lib")

/* copy-pasted from vulkan/vulkan.h so we are not forced to include Windows.h */
#ifndef VK_KHR_WIN32_SURFACE_EXTENSION_NAME /* redefenition guard */
#define VK_KHR_WIN32_SURFACE_EXTENSION_NAME "VK_KHR_win32_surface"

typedef VkFlags VkWin32SurfaceCreateFlagsKHR;

typedef struct VkWin32SurfaceCreateInfoKHR {
    VkStructureType                 sType;
    const void*                     pNext;
    VkWin32SurfaceCreateFlagsKHR    flags;
    HINSTANCE                       hinstance;
    HWND                            hwnd;
} VkWin32SurfaceCreateInfoKHR;


typedef VkResult (VKAPI_PTR *PFN_vkCreateWin32SurfaceKHR)(VkInstance instance, const VkWin32SurfaceCreateInfoKHR* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkSurfaceKHR* pSurface);
typedef VkBool32 (VKAPI_PTR *PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR)(VkPhysicalDevice physicalDevice, uint32_t queueFamilyIndex);
#endif  /* VK_KHR_WIN32_SURFACE_EXTENSION_NAME */

#ifndef GVVK__PLATFORM_VK_FUNCTIONS
#define GVVK__PLATFORM_VK_FUNCTIONS(_)      \
    _(CreateWin32SurfaceKHR)                \

#endif

#endif  /* _WIN32 */

#ifndef GVVK_USER_REQUESTED_FUNCTIONS
#define GVVK_USER_REQUESTED_FUNCTIONS(_)
#endif

#ifndef GVVK__VK_FUNCTIONS
#define GVVK__VK_FUNCTIONS(_)                       \
    GVVK_USER_REQUESTED_FUNCTIONS(_)                \
    GVVK__PLATFORM_VK_FUNCTIONS(_)                  \
 /* _(CreateInstance)                            */ \
 /* _(DestroyInstance)                           */ \
 /* _(EnumeratePhysicalDevices)                  */ \
 /* _(CreateDevice)                              */ \
 /* _(DestroyDevice)                             */ \
    _(GetPhysicalDeviceFeatures)                    \
    _(GetPhysicalDeviceFormatProperties)            \
    _(GetPhysicalDeviceImageFormatProperties)       \
    _(GetPhysicalDeviceProperties)                  \
    _(GetPhysicalDeviceQueueFamilyProperties)       \
    _(GetPhysicalDeviceMemoryProperties)            \
    _(EnumerateInstanceExtensionProperties)         \
    _(EnumerateDeviceExtensionProperties)           \
    _(EnumerateInstanceLayerProperties)             \
    _(EnumerateDeviceLayerProperties)               \
    _(GetDeviceQueue)                               \
    _(QueueSubmit)                                  \
    _(QueueWaitIdle)                                \
    _(DeviceWaitIdle)                               \
    _(AllocateMemory)                               \
    _(FreeMemory)                                   \
    _(MapMemory)                                    \
    _(UnmapMemory)                                  \
    _(FlushMappedMemoryRanges)                      \
    _(InvalidateMappedMemoryRanges)                 \
    _(GetDeviceMemoryCommitment)                    \
    _(BindBufferMemory)                             \
    _(BindImageMemory)                              \
    _(GetBufferMemoryRequirements)                  \
    _(GetImageMemoryRequirements)                   \
    _(GetImageSparseMemoryRequirements)             \
    _(GetPhysicalDeviceSparseImageFormatProperties) \
    _(QueueBindSparse)                              \
    _(CreateFence)                                  \
    _(DestroyFence)                                 \
    _(ResetFences)                                  \
    _(GetFenceStatus)                               \
    _(WaitForFences)                                \
    _(CreateSemaphore)                              \
    _(DestroySemaphore)                             \
    _(CreateEvent)                                  \
    _(DestroyEvent)                                 \
    _(GetEventStatus)                               \
    _(SetEvent)                                     \
    _(ResetEvent)                                   \
    _(CreateQueryPool)                              \
    _(DestroyQueryPool)                             \
    _(GetQueryPoolResults)                          \
    _(CreateBuffer)                                 \
    _(DestroyBuffer)                                \
    _(CreateBufferView)                             \
    _(DestroyBufferView)                            \
    _(CreateImage)                                  \
    _(DestroyImage)                                 \
    _(GetImageSubresourceLayout)                    \
    _(CreateImageView)                              \
    _(DestroyImageView)                             \
    _(CreateShaderModule)                           \
    _(DestroyShaderModule)                          \
    _(CreatePipelineCache)                          \
    _(DestroyPipelineCache)                         \
    _(GetPipelineCacheData)                         \
    _(MergePipelineCaches)                          \
    _(CreateGraphicsPipelines)                      \
    _(CreateComputePipelines)                       \
    _(DestroyPipeline)                              \
    _(CreatePipelineLayout)                         \
    _(DestroyPipelineLayout)                        \
    _(CreateSampler)                                \
    _(DestroySampler)                               \
    _(CreateDescriptorSetLayout)                    \
    _(DestroyDescriptorSetLayout)                   \
    _(CreateDescriptorPool)                         \
    _(DestroyDescriptorPool)                        \
    _(ResetDescriptorPool)                          \
    _(AllocateDescriptorSets)                       \
    _(FreeDescriptorSets)                           \
    _(UpdateDescriptorSets)                         \
    _(CreateFramebuffer)                            \
    _(DestroyFramebuffer)                           \
    _(CreateRenderPass)                             \
    _(DestroyRenderPass)                            \
    _(GetRenderAreaGranularity)                     \
    _(CreateCommandPool)                            \
    _(DestroyCommandPool)                           \
    _(ResetCommandPool)                             \
    _(AllocateCommandBuffers)                       \
    _(FreeCommandBuffers)                           \
    _(BeginCommandBuffer)                           \
    _(EndCommandBuffer)                             \
    _(ResetCommandBuffer)                           \
    _(CmdBindPipeline)                              \
    _(CmdSetViewport)                               \
    _(CmdSetScissor)                                \
    _(CmdSetLineWidth)                              \
    _(CmdSetDepthBias)                              \
    _(CmdSetBlendConstants)                         \
    _(CmdSetDepthBounds)                            \
    _(CmdSetStencilCompareMask)                     \
    _(CmdSetStencilWriteMask)                       \
    _(CmdSetStencilReference)                       \
    _(CmdBindDescriptorSets)                        \
    _(CmdBindIndexBuffer)                           \
    _(CmdBindVertexBuffers)                         \
    _(CmdDraw)                                      \
    _(CmdDrawIndexed)                               \
    _(CmdDrawIndirect)                              \
    _(CmdDrawIndexedIndirect)                       \
    _(CmdDispatch)                                  \
    _(CmdDispatchIndirect)                          \
    _(CmdCopyBuffer)                                \
    _(CmdCopyImage)                                 \
    _(CmdBlitImage)                                 \
    _(CmdCopyBufferToImage)                         \
    _(CmdCopyImageToBuffer)                         \
    _(CmdUpdateBuffer)                              \
    _(CmdFillBuffer)                                \
    _(CmdClearColorImage)                           \
    _(CmdClearDepthStencilImage)                    \
    _(CmdClearAttachments)                          \
    _(CmdResolveImage)                              \
    _(CmdSetEvent)                                  \
    _(CmdResetEvent)                                \
    _(CmdWaitEvents)                                \
    _(CmdPipelineBarrier)                           \
    _(CmdBeginQuery)                                \
    _(CmdEndQuery)                                  \
    _(CmdResetQueryPool)                            \
    _(CmdWriteTimestamp)                            \
    _(CmdCopyQueryPoolResults)                      \
    _(CmdPushConstants)                             \
    _(CmdBeginRenderPass)                           \
    _(CmdNextSubpass)                               \
    _(CmdEndRenderPass)                             \
    _(CmdExecuteCommands)                           \
                                                    \
    _(DestroySurfaceKHR)                            \
    _(GetPhysicalDeviceSurfaceSupportKHR)           \
    _(GetPhysicalDeviceSurfaceCapabilitiesKHR)      \
    _(GetPhysicalDeviceSurfaceFormatsKHR)           \
    _(GetPhysicalDeviceSurfacePresentModesKHR)      \

#endif

typedef struct GvVkFunctionTable {
#define table_record(name) PFN_vk##name name;

    table_record(GetInstanceProcAddr)
    table_record(GetDeviceProcAddr)
    
    table_record(CreateInstance)
    table_record(DestroyInstance)
    table_record(EnumeratePhysicalDevices)
    table_record(CreateDevice)
    table_record(DestroyDevice)

#ifdef GVVK_DEBUG
    table_record(CreateDebugReportCallbackEXT)
    table_record(DestroyDebugReportCallbackEXT)
#endif

GVVK__VK_FUNCTIONS(table_record)
#undef table_record

} GvVkFunctionTable;



#if defined(GVVK_DEBUG) && !defined(GVVK__RESULTS)
#define GVVK__RESULTS(_)                      \
    _(VK_NOT_READY)                           \
    _(VK_TIMEOUT)                             \
    _(VK_EVENT_SET)                           \
    _(VK_EVENT_RESET)                         \
    _(VK_INCOMPLETE)                          \
    _(VK_ERROR_OUT_OF_HOST_MEMORY)            \
    _(VK_ERROR_OUT_OF_DEVICE_MEMORY)          \
    _(VK_ERROR_INITIALIZATION_FAILED)         \
    _(VK_ERROR_DEVICE_LOST)                   \
    _(VK_ERROR_MEMORY_MAP_FAILED)             \
    _(VK_ERROR_LAYER_NOT_PRESENT)             \
    _(VK_ERROR_EXTENSION_NOT_PRESENT)         \
    _(VK_ERROR_FEATURE_NOT_PRESENT)           \
    _(VK_ERROR_INCOMPATIBLE_DRIVER)           \
    _(VK_ERROR_TOO_MANY_OBJECTS)              \
    _(VK_ERROR_FORMAT_NOT_SUPPORTED)          \
    _(VK_ERROR_FRAGMENTED_POOL)               \
    _(VK_ERROR_SURFACE_LOST_KHR)              \
    _(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR)      \
    _(VK_SUBOPTIMAL_KHR)                      \
    _(VK_ERROR_OUT_OF_DATE_KHR)               \
    _(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR)      \
    _(VK_ERROR_VALIDATION_FAILED_EXT)         \
    _(VK_ERROR_INVALID_SHADER_NV)
#endif

#if !defined(GVVK_CHECK) && defined(GVVK_DEBUG)

GVVK_FORCE_INLINE void gvvk_check(VkResult r, const char *op) {
    if (r == VK_SUCCESS) return;
    GVVK__LOG(op);
#define check_case(e) case e: GVVK__LOG(": returned " #e "\n"); break;
    switch (r) {
        GVVK__RESULTS(check_case)
        default: GVVK__LOG(": returned UNKNOWN (NOT VK_SUCCESS\n)"); break;
    }
#undef check_case
    GVVK__BREAK();
}

#define GVVK_CHECK(op) gvvk_check((op), #op)
#endif

#if !defined(GVVK_CHECK)
#define GVVK_CHECK(op) op /* fallback */
#endif

#ifndef GVVK_MAKE_INFO
#define GVVK_MAKE_INFO GVVK_FORCE_INLINE
#endif

typedef struct GvVkQueue {
    uint32_t family;
    VkQueue queue;
} GvVkQueue;

typedef struct GvVkContext {
#ifdef _WIN32
    HMODULE vulkan_dll;

    HINSTANCE hinst;
    HWND hwnd;
    HDC hdc;
#endif

    GvVkFunctionTable ft;
    
    VkInstance inst;
    VkPhysicalDevice phdev;
    VkDevice dev;

    VkSurfaceKHR surface;

    GvVkQueue graphics;
    GvVkQueue compute;
    GvVkQueue transfer;
    GvVkQueue present;

#ifdef GVVK_DEBUG
    VkDebugReportCallbackEXT debug_cb;
#endif

} GvVkContext;

#ifdef _WIN32
GVVK_API void gvvkInitContext(GvVkContext *ctx, HINSTANCE hinst, int cmd_show);
#else
#error Linux is not yet supported
#endif

GVVK_API void gvvkDestroyContext(GvVkContext *ctx);


#endif  /* __GV_VULKAN__ */

#ifdef GVVK_IMPLEMENTATION

GVVK_FORCE_INLINE gvvk_minu(unsigned a, unsigned b) { return a < b ? a : b; }

GVVK_INTERN gvvk_debugCallback(VkDebugReportFlagsEXT f, VkDebugReportObjectTypeEXT type, uint64_t obj, size_t loc, int32_t code, const char* prefix, const char* msg, void* ud) {
    GVVK__LOG("[");
    GVVK__LOG(prefix);
    GVVK__LOG("] ");
    GVVK__LOG(msg);
    GVVK__LOG("\n");
}

#ifdef _WIN32
GVVK_INTERN LRESULT gvvk_wndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;

    }
    return DefWindowProcA(hwnd, msg, wparam, lparam);
}

GVVK_INTERN void gvvk_createSurface(GvVkContext *ctx, HINSTANCE hinst, int cmd_show) {
    static const char *class_name = "GVVK_WINDOW_CLASS";

    WNDCLASSA wc = { 0 };
    wc.lpszClassName = class_name;
    wc.hInstance = hinst;
    wc.lpfnWndProc = &gvvk_wndProc;

    RegisterClassA(&wc);


    ctx->hwnd = CreateWindowExA(0, class_name, "GVVK", 
                                WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_THICKFRAME | WS_VISIBLE,
                                100, 100, 1280, 720,
                                NULL, NULL, hinst, NULL);

    ctx->hdc = GetDC(ctx->hwnd);
/*
    PIXELFORMATDESCRIPTOR pfd;
    ZeroMemory(&pfd, sizeof(pfd));
    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;

    int pf = ChoosePixelFormat(ctx->hdc, &pfd);
    SetPixelFormat(ctx->hdc, pf, &pfd);
    DescribePixelFormat(ctx->hdc, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

*/
    ShowWindow(ctx->hwnd, cmd_show);
    UpdateWindow(ctx->hwnd);
    
    GvVkFunctionTable *ft = &ctx->ft;
    VkWin32SurfaceCreateInfoKHR surface_ci = { 0 };
    surface_ci.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    surface_ci.hinstance = hinst;
    surface_ci.hwnd = ctx->hwnd;

    GVVK_CHECK(ft->CreateWin32SurfaceKHR(ctx->inst, &surface_ci, VK_NULL_HANDLE, &ctx->surface));
}
#endif  /* _WIN32 */

GVVK_API void gvvkInitContext(GvVkContext *ctx, HINSTANCE hinst, int cmd_show) {
    GvVkFunctionTable *ft = &ctx->ft;
    ctx->inst = VK_NULL_HANDLE;

#ifdef _WIN32
    ctx->vulkan_dll = LoadLibraryA("vulkan-1.dll");
#define get_dyn_proc(name) (ft->name = (PFN_vk##name) GetProcAddress(ctx->vulkan_dll, "vk" #name))
#else
#error Linux not yet supported
#endif


#define get_inst_proc(name) (ft->name = (PFN_vk##name) ft->GetInstanceProcAddr(ctx->inst, "vk" #name))
#define get_dev_proc(name) (ft->name = (PFN_vk##name) ft->GetDeviceProcAddr(ctx->dev, "vk" #name))
    
/* when release load driver functions first, then if nessesery use loaders one */
#ifdef GVVK_DEBUG 
#define get_proc(name) if (!get_inst_proc(name)) if (!ctx->dev || !get_dev_proc(name)) if (!get_dyn_proc(name)) __debugbreak();
#else
#define get_proc(name) if (!get_dev_proc(name)) if (!get_inst_proc(name)) get_dyn_proc(name);
#endif
    
    get_dyn_proc(GetInstanceProcAddr);
    get_dyn_proc(GetDeviceProcAddr);

    get_inst_proc(CreateInstance);

    VkApplicationInfo ai = { 0 };
    ai.apiVersion = VK_MAKE_VERSION(1, 0, 39);
    ai.pEngineName = "GVVK";
    ai.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    ai.pApplicationName = "Some shitty thingy";
    ai.applicationVersion = VK_MAKE_VERSION(0, 0, 0);

    VkInstanceCreateInfo inst_ci = { 0 };
    inst_ci.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    inst_ci.pApplicationInfo = &ai;

#ifdef GVVK_DEBUG
    static const char *layer_names[] = { "VK_LAYER_LUNARG_standard_validation", };
    inst_ci.ppEnabledLayerNames = layer_names;
    inst_ci.enabledLayerCount = GVVK__ARR_SIZE(layer_names);
#endif
    static const char *extenstion_names[] = { 
#ifdef GVVK_DEBUG
        VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef _WIN32
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#else
#error Linux not yet supported
#endif
    };

    inst_ci.ppEnabledExtensionNames = extenstion_names;
    inst_ci.enabledExtensionCount = GVVK__ARR_SIZE(extenstion_names);

    GVVK_CHECK(ft->CreateInstance(&inst_ci, VK_NULL_HANDLE, &ctx->inst));

#ifdef GVVK_DEBUG
    get_inst_proc(CreateDebugReportCallbackEXT);
    get_inst_proc(DestroyDebugReportCallbackEXT);

    VkDebugReportCallbackCreateInfoEXT debug_report_ci = {0};
    debug_report_ci.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
    debug_report_ci.flags = VK_DEBUG_REPORT_FLAG_BITS_MAX_ENUM_EXT;
    debug_report_ci.pfnCallback = &gvvk_debugCallback;

    GVVK_CHECK(ft->CreateDebugReportCallbackEXT(ctx->inst, &debug_report_ci, NULL, &ctx->debug_cb));
#endif

    get_inst_proc(DestroyInstance);
    get_inst_proc(EnumeratePhysicalDevices);
    get_inst_proc(CreateDevice);

    get_inst_proc(GetPhysicalDeviceQueueFamilyProperties);
    get_inst_proc(GetPhysicalDeviceSurfaceSupportKHR);
    get_inst_proc(CreateWin32SurfaceKHR);

    VkPhysicalDevice physical_devices[16] = { 0 };
    uint32_t physical_device_count = 0;
    GVVK_CHECK(ft->EnumeratePhysicalDevices(ctx->inst, &physical_device_count, VK_NULL_HANDLE));
    physical_device_count = GVVK__ARR_SIZE(physical_devices);
    GVVK_CHECK(ft->EnumeratePhysicalDevices(ctx->inst, &physical_device_count, physical_devices));

    ctx->phdev = physical_devices[0];
    
#ifdef _WIN32
    gvvk_createSurface(ctx, hinst, cmd_show);
#else
#error Linux is not yet supported
#endif

    /* I fucking love hard coding */
    uint32_t gq = -1, cq = -1, tq = -1, pq = -1; /* graphics queue, compute queue, tranfer queue, present queue */

                                                     
    VkQueueFamilyProperties queue_props[32];
    VkBool32 present_support;

    uint32_t queue_props_count = sizeof(queue_props) / sizeof(queue_props[0]);

    ft->GetPhysicalDeviceQueueFamilyProperties(ctx->phdev, &queue_props_count, queue_props);

#define has_flag(x, flag) (((x) & (flag)) == (flag))
#define change(what, other1, other2, other3, to) (((what) == (other1) || (what) == (other2) || (what) == (other3)) && ((to) != (other1) && (to) != (other2) && (to) != (other3))) 
 
    unsigned i; 
    for (i = 0; i < queue_props_count; i++) {
        uint32_t qf = queue_props[i].queueFlags;

        if ((gq == -1 || change(gq, cq, tq, pq, i)) && has_flag(qf, VK_QUEUE_GRAPHICS_BIT))
            gq = i;

        if ((cq == -1 || change(cq, gq, tq, pq, i)) && has_flag(qf, VK_QUEUE_COMPUTE_BIT))
            cq = i;
        
        if ((tq == -1 || change(tq, gq, cq, pq, i)) && has_flag(qf, VK_QUEUE_TRANSFER_BIT))
            tq = i;

        GVVK_CHECK(ft->GetPhysicalDeviceSurfaceSupportKHR(ctx->phdev, i, ctx->surface, &present_support));
        if ((pq == -1 || change(pq, gq, cq, tq, i)) && present_support)
            pq  = i;
    }

#undef change
#undef has_flag
    
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

    VkDeviceCreateInfo dev_ci = { 0 };
    dev_ci.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    dev_ci.enabledExtensionCount = 1;
    dev_ci.ppEnabledExtensionNames = (const char *[]) { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    dev_ci.pQueueCreateInfos = queue_cis;
    dev_ci.queueCreateInfoCount = queue_count;

    /* pick first (hope it's descreate) */
    GVVK_CHECK(ft->CreateDevice(ctx->phdev, &dev_ci, VK_NULL_HANDLE, &ctx->dev));

    get_dev_proc(DestroyDevice);
    GVVK__VK_FUNCTIONS(get_proc)

    if (gq != -1) ft->GetDeviceQueue(ctx->dev, gq, gvvk_minu(gq_idx, queue_props[gq].queueCount - 1), &ctx->graphics.queue);
    if (cq != -1) ft->GetDeviceQueue(ctx->dev, cq, gvvk_minu(cq_idx, queue_props[cq].queueCount - 1), &ctx->compute.queue);
    if (tq != -1) ft->GetDeviceQueue(ctx->dev, tq, gvvk_minu(tq_idx, queue_props[tq].queueCount - 1), &ctx->transfer.queue);
    if (pq != -1) ft->GetDeviceQueue(ctx->dev, pq, gvvk_minu(pq_idx, queue_props[pq].queueCount - 1), &ctx->present.queue);

    ctx->graphics.family = gq;
    ctx->compute.family = cq;
    ctx->transfer.family = tq;
    ctx->present.family = pq;

    /* someshow remove this */
    get_inst_proc(DestroySurfaceKHR);

#undef get_proc
#undef get_dyn_proc
#undef get_dev_proc
#undef get_inst_proc
}

GVVK_API void gvvkDestroyContext(GvVkContext *ctx) {
    GvVkFunctionTable *ft = &ctx->ft;
    
    ft->DestroySurfaceKHR(ctx->inst, ctx->surface, VK_NULL_HANDLE);
    DestroyWindow(ctx->hwnd);
    
    ft->DestroyDevice(ctx->dev, VK_NULL_HANDLE);
#ifdef GVVK_DEBUG
    ft->DestroyDebugReportCallbackEXT(ctx->inst, ctx->debug_cb, VK_NULL_HANDLE);
#endif

    ft->DestroyInstance(ctx->inst, VK_NULL_HANDLE);
    FreeLibrary(ctx->vulkan_dll);
}

#endif  /* GVVK_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif