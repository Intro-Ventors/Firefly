#pragma once

#include "Utility.hpp"

#include <optional>
#include <vector>

namespace Firefly
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
		explicit Queue(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag)
			: m_vFlags(vFlag)
		{
			// Get the queue family count.
			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, nullptr);

			// Get the queue family properties.
			std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, queueFamilies.data());

			// Iterate over those queue family properties and find the most suitable one.
			for (uint32_t family = 0; family < queueFamilyCount; family++)
			{
				// Check if the queue flag contains what we want. If so, we can assign the queue family and return from the function.
				if (queueFamilies[family].queueCount > 0 && queueFamilies[family].queueFlags & vFlag)
				{
					m_QueueFamily = family;
					return;
				}
			}

			// Throw a runtime error if a queue wasn't found.
			throw BackendError("A queue wasn't found with the required flags!");
		}

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

		/**
		 * Get the Vulkan queue address.
		 *
		 * @return The queue pointer.
		 */
		const VkQueue* getQueueAddr() const { return &m_vQueue; }

		/**
		 * Get the Vulkan queue address.
		 *
		 * @return The queue pointer.
		 */
		VkQueue* getQueueAddr() { return &m_vQueue; }

		/**
		 * Get the queue flags.
		 *
		 * @return The queue flags.
		 */
		VkQueueFlagBits getFlags() const { return m_vFlags; }

	private:
		std::optional<uint32_t> m_QueueFamily = {};
		VkQueue m_vQueue = VK_NULL_HANDLE;
		VkQueueFlagBits m_vFlags;
	};
}