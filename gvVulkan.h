/*
    gvVulkan.h - private code - Bart³omiej's utilities made for vulkan 

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
#define GVVK__RESULTS(_)                 \
    _(VK_NOT_READY)                      \
    _(VK_TIMEOUT)                        \
    _(VK_EVENT_SET)                      \
    _(VK_EVENT_RESET)                    \
    _(VK_INCOMPLETE)                     \
    _(VK_ERROR_OUT_OF_HOST_MEMORY)       \
    _(VK_ERROR_OUT_OF_DEVICE_MEMORY)     \
    _(VK_ERROR_INITIALIZATION_FAILED)    \
    _(VK_ERROR_DEVICE_LOST)              \
    _(VK_ERROR_MEMORY_MAP_FAILED)        \
    _(VK_ERROR_LAYER_NOT_PRESENT)        \
    _(VK_ERROR_EXTENSION_NOT_PRESENT)    \
    _(VK_ERROR_FEATURE_NOT_PRESENT)      \
    _(VK_ERROR_INCOMPATIBLE_DRIVER)      \
    _(VK_ERROR_TOO_MANY_OBJECTS)         \
    _(VK_ERROR_FORMAT_NOT_SUPPORTED)     \
    _(VK_ERROR_FRAGMENTED_POOL)          \
    _(VK_ERROR_SURFACE_LOST_KHR)         \
    _(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR) \
    _(VK_SUBOPTIMAL_KHR)                 \
    _(VK_ERROR_OUT_OF_DATE_KHR)          \
    _(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR) \
    _(VK_ERROR_VALIDATION_FAILED_EXT)    \
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

GVVK_MAKE_INFO void gvvkApplicationInfo(VkApplicationInfo *app_info) {
    app_info->sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
}


#endif  /* __GV_VULKAN__ */

#ifdef GVVK_IMPLEMENTATION

#endif  /* GVVK_IMPLEMENTATION */

#ifdef __cplusplus
}
#endif