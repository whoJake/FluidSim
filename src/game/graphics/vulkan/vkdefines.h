#pragma once
#define GFX_SUPPORTS_VULKAN

#ifdef GFX_SUPPORTS_VULKAN

#define VMA_STATIC_VULKAN_FUNCTIONS 1
#include "vk_mem_alloc.h"

#define VK_ASSERT_RESULT(result, msg, ...) do{ if(result != VK_SUCCESS){ QUITFMT(msg, __VA_ARGS__); } }while(0)

#endif // GFX_SUPPORTS_VULKAN