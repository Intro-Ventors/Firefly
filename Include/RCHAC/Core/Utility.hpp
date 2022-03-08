#pragma once

#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

#include <string>

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

		/**
		 * Validate the incoming result.
		 * This will throw an exception depending on the result provided. If the result is VK_SUCCESS, it will not do anything.
		 *
		 * @param result The result to be validated.
		 * @param string The output message.
		 * @throws std::runtime_error depending on the result.
		 */
		void ValidateResult(const VkResult result, const std::string_view& string);
	}
}