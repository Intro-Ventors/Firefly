#include "RCHAC/Core/Queue.hpp"

#include <stdexcept>
#include <vector>

namespace RCHAC
{
	Queue::Queue(VkPhysicalDevice vPhysicalDevice, VkQueueFlagBits vFlag)
	{
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, nullptr);

		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(vPhysicalDevice, &queueFamilyCount, queueFamilies.data());

		int32_t i = 0;
		for (auto itr = queueFamilies.begin(); itr != queueFamilies.end(); itr++, i++)
		{
			if (itr->queueCount > 0)
			{
				// Check if the queue flag contains what we want.
				if (itr->queueFlags & vFlag)
					m_QueueFamily = i;

				// Escape from the loop if the queues were found.
				if (isComplete())
					return;
			}
		}

		throw std::runtime_error("A queue wasn't found with the required flags!");
	}
	
	void Queue::setupQueue(VkDevice vLogicalDevice)
	{
		vkGetDeviceQueue(vLogicalDevice, m_QueueFamily.value(), 0, &m_vQueue);
	}
}