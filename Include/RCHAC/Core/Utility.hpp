#pragma once

#include <vulkan/vulkan.h>

namespace RCHAC
{
	namespace Utility
	{
		/**
		 * Validate the incoming result.
		 * This will throw an exception depending on the result provided. If the result is VK_SUCCESS, it will not do anything.
		 *
		 * @param result The result to be validated.
		 * @throws std::runtime_error depending on the result.
		 */
		void ValidateResult(const VkResult result);
	}
}