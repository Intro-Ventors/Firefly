#pragma once


#ifndef VOLK_IMPLEMENTATION

#define VOLK_IMPLEMENTATION
#define VK_ENABLE_BETA_EXTENSIONS
#define VK_NO_PROTOTYPES
#define VMA_IMPLEMENTATION

#include <volk/volk.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

#endif

#include "BackendError.hpp"

namespace Firefly
{
	namespace Utility
	{
		/**
		 * Validate the incoming result.
		 * This will throw an exception depending on the result provided. If the result is VK_SUCCESS, it will not do anything.
		 *
		 * @param result The result to be validated.
		 * @param string The output message.
		 * @throws std::runtime_error depending on the result.
		 */
		inline void ValidateResult(const VkResult result, const std::string_view& string)
		{
			if (result != VkResult::VK_SUCCESS)
				throw BackendError(string);
		}
	}
}

#define FIREFLY_DEFAULT_COPY(name)	name(const name&) = default;	name& operator=(const name&) = default
#define FIREFLY_DEFAULT_MOVE(name)	name(name&&) = default;			name& operator=(name&&) = default