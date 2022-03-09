#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#define VK_NO_PROTOTYPES

#ifdef FIREFLY_SETUP_THIRD_PARTY
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#include <SPIRV-Reflect/spirv_reflect.c>

#endif

#include <volk/volk.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>