/*
    gvvk.h - private code - Bart³omiej's utilities made for vulkan 

ABOUT:
    Simple library made to make using Vulkan more pleasant.

DEFINE:
    GVVK_IMPLEMENTATION - to define functions

    GVVK_DEBUG - to add some checks and debugging

    GVVK_IGNORE_GV_DEBUG - to ignore GV_DEBUG
    
    GVVK_DOWN_FORCE_INLINE - to not force inline

    GV_DEBUG - to define GVVK_DEBUG automaticly

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

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#pragma comment(lib, "vulkan-1.lib")
#define VK_USE_PLATFORM_WIN32_KHR

#endif  /* _WIN32 */

#include <stdint.h>
#include <vulkan/vulkan.h>

#if !defined(GVVK_DEBUG) && defined(GV_DEBUG) && !defined(GVVK_IGNORE_GV_DEBUG)
#define GVVK_DEBUG GV_DEBUG
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

#if defined(GVVK_DEBUG) && !defined(GVVK__LOG)
#ifdef _WIN32
#define GVVK__LOG OutputDebugStringA
#else
#include <stdio.h>
#define GVVK__LOG puts
#endif
#endif

#ifndef GVVK__BREAK
#ifdef _WIN32
#define GVVK__BREAK() __debugbreak()
#else
#define GVVK__BREAK() asm("int $3")
#endif
#endif

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

#ifndef GVVK__HANDLES
#define GVVK__HANDLES(_)                                                                          \
    _( 1, Semaphore,            semephore,         vkDestroySemaphore,                     0)     \
    _( 2, Fence,                fence,             vkDestroyFence,                         0)     \
    _( 3, DeviceMemory,         dev_mem,           vkFreeMemory,                           0)     \
    _( 4, Buffer,               buff,              vkDestroyBuffer,                        0)     \
    _( 5, Image,                img,               vkDestroyImage,                         0)     \
 /* _( 6, Event,                event,             vkDestroyEvent,                         0) */  \
 /* _( 7, QueryPool,            query_pool,        vkDestroyQueryPool,                     0) */  \
 /* _( 8, BufferView,           buff_view,         vkDestroyBufferView,                    0) */  \
    _( 9, ImageView,            img_view,          vkDestroyImageView,                     0)     \
    _(10, ShaderModule,         shader,            vkDestroyShaderModule,                  0)     \
 /* _(11, PipelineCache,        pipeline_cache,    vkDestroyPipelineCache,                 0) */  \
    _(12, PipelineLayout,       pipeline_layout,   vkDestroyPipelineLayout,                0)     \
    _(13, RenderPass,           render_pass,       vkDestroyRenderPass,                    0)     \
    _(14, Pipeline,             pipeline,          vkDestroyPipeline,                      0)     \
    _(15, DescriptorSetLayout,  desc_set_layout,   vkDestroyDescriptorSetLayout,           0)     \
    _(16, Sampler,              sampler,           vkDestroySampler,                       0)     \
    _(17, DescriptorPool,       desc_pool,         vkDestroyDescriptorPool,                0)     \
 /* _(18, DescriptorSet,        desc_set,          vkFreeDescriptorSets,                   0) */  \
    _(19, Framebuffer,          framebuffer,       vkDestroyFramebuffer,                   0)     \
    _(20, CommandPool,          cmd_pool,          vkDestroyCommandPool,                   0)
#endif

typedef enum GvVkHandleType {
    GvVkHandleType_None = 0,
#define def_enum(id, type, name, destroy, ign) GvVkHandleType_##type = id,
    GVVK__HANDLES(def_enum)
#undef def_enum
    GvVkHandleType_MaxEnum 
} GvVkHandleType;

typedef struct GvVk_HandleTypeInfo {
    const GvVkHandleType type;
    const char *const type_name;
    union {
        const void (*destroy_handle_fn)(VkDevice, void*, VkAllocationCallbacks*);
#define def_destroy(id, type, name, destroy, ign) const PFN_##destroy destroy_##name##_fn;
        GVVK__HANDLES(def_destroy)
#undef def_destroy
    };
} GvVk_HandleTypeInfo;

const GvVk_HandleTypeInfo gvvk__handle_type_infos[] = {
    [GvVkHandleType_None] = { GvVkHandleType_None, NULL, NULL },
#define def_info(id, type, name, destroy, ign) [GvVkHandleType_##type] = { GvVkHandleType_##type, #type, &destroy },
    GVVK__HANDLES(def_info)
#undef def_info
};

typedef struct GvVkHandle {
    const GvVkHandleType htype;
    union {
        void *handle;
#define def_handle(id, type, name, destroy, ign) Vk##type name;
        GVVK__HANDLES(def_handle)
#undef def_handle
    };
} GvVkHandle;


#ifndef GVVK_INIT_HANDLE
#define GVVK_INIT_HANDLE(type) ((GvVkHandle) { GvVkHandleType_##type, VK_NULL_HANDLE })
#endif

#ifndef GVVK_HANDLE
#define GVVK_HANDLE(type, name) GvVkHandle name = GVVK_INIT_HANDLE(type)
#endif

#ifndef gvvkDestroyHandle
#define gvvkDestroyHandle(dev, handle, alloc_cb)                                            \
    do {                                                                                    \
        gvvk__handle_type_infos[(handle)->htype]((dev), (handle)->handle, (alloc_cb));      \
        (handle)->htype = GvVkHandleType_None;                                              \
        (handle)->handle = NULL;                                                            \
    } while (0)
#endif

GVVK_MAKE_INFO VkApplicationInfo gvvkApplicationInfo(const char *app_name) {
    VkApplicationInfo ai = {0};
    ai.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    ai.applicationVersion = app_name;
    return ai;
}


#endif  /* __GV_VULKAN__ */

#ifdef GVVK_IMPLEMENTATION

#endif  /* GVVK_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif