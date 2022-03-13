#include "Firefly/Queue.hpp"

#include <vector>

namespace Firefly
{
	Queue::Queue(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag)
		: m_vFlags(vFlag)
	{
		// Get the queue family count.
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, nullptr);

		// Get the queue family properties.
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, queueFamilies.data());

		// Iterate over those queue family properties and find the most suitable one.
		for (uint32_t index = 0; index < queueFamilyCount; index++)
		{
			const auto& family = queueFamilies[index];

			// Check if the queue flag contains what we want. If so, we can assign the queue family and return from the function.
			if (family.queueCount > 0 && family.queueFlags & vFlag)
			{
				m_QueueFamily = index;
				return;
			}
		}

		// Throw a runtime error if a queue wasn't found.
		throw BackendError("A queue wasn't found with the required flags!");
	}
}