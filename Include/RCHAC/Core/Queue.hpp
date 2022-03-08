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
		 * Default constructor.
		 */
		Queue() = default;

		/**
		 * Constructor.
		 * This constructor will not create the VkQueue itself, rather it will find only the required queue family.
		 *
		 * @param vPhysicalDevice The physical device to which the queue is bound to.
		 * @param vFlag The queue flags.
		 * @throws std::runtime_error If no queue was found.
		 */
		Queue(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag);

		/**
		 * Create the queue object using the logical device.
		 *
		 * @param vLogicalDevice The logical device to which the queue is bound to.
		 */
		void setupQueue(VkDevice vLogicalDevice);

		/**
		 * Check if the queue is complete.
		 *
		 * @return Boolean value.
		 */
		bool isComplete() const { return m_QueueFamily.has_value(); }

		/**
		 * Get the queue family of the queue.
		 *
		 * @return The optional queue family.
		 */
		std::optional<uint32_t> getFamily() const { return m_QueueFamily; }

		/**
		 * Get the Vulkan queue.
		 *
		 * @return The queue.
		 */
		VkQueue getQueue() const { return m_vQueue; }

	private:
		std::optional<uint32_t> m_QueueFamily = {};
		VkQueue m_vQueue = VK_NULL_HANDLE;
	};
}