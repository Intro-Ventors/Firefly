#include "RCHAC/Core/Engine.hpp"
#include "RCHAC/Core/Instance.hpp"
#include "RCHAC/Core/Queue.hpp"

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
			uint32_t extensionCount = 0;
			vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, nullptr);

			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, availableExtensions.data());

			std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

			for (const VkExtensionProperties& extension : availableExtensions)
				requiredExtensions.erase(extension.extensionName);

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
			const bool extensionsSupported = CheckDeviceExtensionSupport(vPhysicalDevice, deviceExtensions);
			Queue vQueue(vPhysicalDevice, flags);

			return vQueue.isComplete() && extensionsSupported;
		}
	}

	Engine::Engine(const std::shared_ptr<Instance>& pInstance, const VkQueueFlagBits flag)
		: m_pInstance(pInstance)
	{
		// Validate the pointer.
		if (!m_pInstance)
			throw std::runtime_error("The instance pointer should not be null!");

		const auto vInstance = m_pInstance->getInstance();

		// Enumerate physical devices.
		uint32_t deviceCount = 0;
		Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, nullptr), "Failed to enumerate physical devices.");

		// Throw an error if there are no physical devices available.
		if (deviceCount == 0)
			throw std::runtime_error("No physical devices found!");

		std::vector<VkPhysicalDevice> vCandidates(deviceCount);
		Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, vCandidates.data()));

		// Setup extensions.
		std::vector<const char*> extensions;

		if (flag == VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			extensions.emplace_back("VK_EXT_video_encode_h264");
		else if (flag == VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			extensions.emplace_back("VK_EXT_video_decode_h264");

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

			m_vPhysicalDevice = VK_NULL_HANDLE;
		}

		// Validate if a physical device was found.
		if (m_vPhysicalDevice == VK_NULL_HANDLE)
			throw std::runtime_error("Unable to find suitable physical device!");
	}

	Engine::~Engine()
	{
		// Destroy the logical device.
		vkDestroyDevice(m_vLogicalDevice, nullptr);
	}
}