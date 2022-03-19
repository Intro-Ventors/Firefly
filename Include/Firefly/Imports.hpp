#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#define VK_NO_PROTOTYPES

#if defined(_WIN32)
#	define VK_USE_PLATFORM_WIN32_KHR

#elif defined(__ANDROID__)
#	define VK_USE_PLATFORM_ANDROID_KHR

#endif

#include <volk/volk.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>