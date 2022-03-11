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
}