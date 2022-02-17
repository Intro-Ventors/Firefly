#pragma once

#include "Utility.hpp"
#include <optional>

namespace RCHAC
{
	/**
	 * RCHAC Queue object.
	 * This object is used to queue commands that are to be executed by the GPU.
	 */
	class Queue final
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param vPhysicalDevice The physical device to which the queue is bound to.
		 * @param vFlag The queue flags.
		 */
		Queue(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag);

		/**
		 * Check if the queue is complete.
		 * 
		 * @return Boolean value.
		 */
		bool isComplete() const { return m_QueueFamily.has_value(); }

	private:
		std::optional<uint32_t> m_QueueFamily = {};
		VkQueue m_vQueue = VK_NULL_HANDLE;
	};
}