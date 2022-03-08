#pragma once

#define VK_NO_PROTOTYPES
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

#ifndef VOLK_IMPLEMENTATION
#define VOLK_IMPLEMENTATION
#include <volk/volk.h>

#endif

#include <string>
#include <stdexcept>

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
				throw std::runtime_error(string.data());
		}
	}
}