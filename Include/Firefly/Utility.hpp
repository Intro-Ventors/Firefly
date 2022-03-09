#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#define VK_NO_PROTOTYPES

#ifdef FIREFLY_SETUP_THIRD_PARTY
#define VOLK_IMPLEMENTATION
#define VMA_IMPLEMENTATION

#endif

#include <volk/volk.h>
#include <VulkanMemoryAllocator/vk_mem_alloc.h>

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

#define FIREFLY_NO_COPY(name)		name(const name&) = delete;		name& operator=(const name&) = delete
#define FIREFLY_NO_MOVE(name)		name(name&&) = delete;			name& operator=(name&&) = delete