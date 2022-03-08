#include "RCHAC/Core/Engine.hpp"
#include "RCHAC/Core/Instance.hpp"

#include <stdexcept>
#include <set>

namespace RCHAC
{
	namespace /* anonymous */
	{
		/**
		 * Check if the device supports the required extensions.
		 *
		 * @param vPhysicalDevice The physical device to check.
		 * @param deviceExtensions The device extensions to check.
		 * @return Boolean stating if its supported or not.
		 */
		bool CheckDeviceExtensionSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions)
		{
			// Get the extension count.
			uint32_t extensionCount = 0;
			Utility::ValidateResult(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, nullptr));

			// Load the extensions.
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			Utility::ValidateResult(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, availableExtensions.data()));

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			// Iterate and check if it contains the extensions we need. If it does, remove them from the set so we can later check if 
			// all the required extensions exist.
			for (const VkExtensionProperties& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

			// If the required extensions set is empty, it means that all the required extensions exist within the physical device.
			return requiredExtensions.empty();
		}

		/**
		 * Check if the physical device is suitable for use.
		 *
		 * @param vPhysicalDevice The physical device to check.
		 * @param deviceExtensions The device extensions to check with.
		 * @param flags The device flags that are needed.
		 * @return Boolean value stating if its viable or not.
		 */
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions, const VkQueueFlagBits flags)
		{
			const Queue vQueue(vPhysicalDevice, flags);
			const bool extensionsSupported = CheckDeviceExtensionSupport(vPhysicalDevice, deviceExtensions);

			return vQueue.isComplete() && extensionsSupported;
		}
	}

	Engine::Engine(const std::shared_ptr<Instance>& pInstance, const VkQueueFlagBits flag)
		: m_pInstance(pInstance)
	{
		// Validate the pointer.
		if (!m_pInstance)
			throw std::runtime_error("The instance pointer should not be null!");

		// Setup extensions.
		std::vector<const char*> extensions = { "VK_KHR_video_queue" , "VK_KHR_synchronization2" };

		if (flag == VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
		{
			extensions.emplace_back("VK_KHR_video_encode_queue");
			extensions.emplace_back("VK_EXT_video_encode_h264");
		}
		else if (flag == VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
		{
			extensions.emplace_back("VK_KHR_video_decode_queue");
			extensions.emplace_back("VK_EXT_video_decode_h264");
		}

		// Create the physical device.
		setupPhysicalDevice(extensions, flag);

		// Create the logical device.
		setupLogicalDevice(extensions, flag);
	}

	Engine::~Engine()
	{
		// Destroy the logical device.
		vkDestroyDevice(m_vLogicalDevice, nullptr);
	}

	void Engine::setupPhysicalDevice(const std::vector<const char*>& extensions, const VkQueueFlagBits flag)
	{
		// Get the Vulkan instance.
		const auto vInstance = m_pInstance->getInstance();

		// Enumerate physical devices.
		uint32_t deviceCount = 0;
		Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, nullptr), "Failed to enumerate physical devices.");

		// Throw an error if there are no physical devices available.
		if (deviceCount == 0)
			throw std::runtime_error("No physical devices found!");

		std::vector<VkPhysicalDevice> vCandidates(deviceCount);
		Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, vCandidates.data()));

		VkPhysicalDeviceProperties vPhysicalDeviceProperties = {};
		// Iterate through all the candidate devices and find the best device.
		for (const VkPhysicalDevice& vCandidateDevice : vCandidates)
		{
			if (IsPhysicalDeviceSuitable(vCandidateDevice, extensions, flag))
			{
				vkGetPhysicalDeviceProperties(vCandidateDevice, &vPhysicalDeviceProperties);

				// Select this as the highest priority.
				if (vPhysicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					m_vPhysicalDevice = vCandidateDevice;
					break;
				}

				// The rest can be with normal priority.
				else if (vPhysicalDeviceProperties.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
					m_vPhysicalDevice = vCandidateDevice;
				else
					m_vPhysicalDevice = vCandidateDevice;
			}
		}

		// Validate if a physical device was found.
		if (m_vPhysicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Unable to find suitable physical device!");
	}

	void Engine::setupLogicalDevice(const std::vector<const char*>& extensions, const VkQueueFlagBits flag)
	{
		// Get the Vulkan instance.
		const auto vInstance = m_pInstance->getInstance();

		// Initialize the queue families.
		m_Queue = Queue(m_vPhysicalDevice, flag);

		// Setup device queues.
		constexpr float priority = 1.0f;
		VkDeviceQueueCreateInfo vQueueCreateInfo = {};
		vQueueCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		vQueueCreateInfo.queueFamilyIndex = m_Queue.getFamily().value();
		vQueueCreateInfo.queueCount = 1;
		vQueueCreateInfo.pQueuePriorities = &priority;

		// Physical device features.
		VkPhysicalDeviceFeatures vFeatures = {};

		// Device create info.
		VkDeviceCreateInfo vDeviceCreateInfo = {};
		vDeviceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		vDeviceCreateInfo.queueCreateInfoCount = 1;
		vDeviceCreateInfo.pQueueCreateInfos = &vQueueCreateInfo;
		vDeviceCreateInfo.pEnabledFeatures = &vFeatures;
		vDeviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		vDeviceCreateInfo.ppEnabledExtensionNames = extensions.data();

		// Get the validation layers and initialize it if validation is enabled.
		const std::vector<const char*> validationLayers = m_pInstance->getValidationLayers();
		if (m_pInstance->isValidationEnabled())
		{
			vDeviceCreateInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			vDeviceCreateInfo.ppEnabledLayerNames = validationLayers.data();
		}
		else
			vDeviceCreateInfo.enabledLayerCount = 0;

		// Create the device.
		Utility::ValidateResult(vkCreateDevice(m_vPhysicalDevice, &vDeviceCreateInfo, nullptr, &m_vLogicalDevice), "Failed to create the logical device!");

		// Setup the queue.
		m_Queue.setupQueue(m_vLogicalDevice);
	}
}