#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#define VK_NO_PROTOTYPES

#ifdef _WIN32
#	define VK_USE_PLATFORM_WIN32_KHR

#endif

#include <volk/volk.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>