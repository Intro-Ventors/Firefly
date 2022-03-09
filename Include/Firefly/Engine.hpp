#pragma once

#include "Instance.hpp"

#include <memory>
#include <set>
#include <map>
#include <array>

namespace Firefly
{
	/**
	 * RCHAC Engine class.
	 * This class is the base class for the three engines, Graphics, Encoder and Decoder.
	 */
	class Engine
	{
	public:
		FIREFLY_DEFAULT_COPY(Engine);
		FIREFLY_DEFAULT_MOVE(Engine);

		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound to.
		 * @param flag The queue flag bits.
		 * @param extensions The device extensions to activate.
		 * @throws std::runtime_error If the pointer is null. It could also throw this same exception if there are no physical devices.
		 */
		explicit Engine(const std::shared_ptr<Instance>& pInstance, const VkQueueFlags flag, const std::vector<const char*>& extensions)
			: m_pInstance(pInstance)
		{
			// Validate the pointer.
			if (!m_pInstance)
				throw BackendError("The instance pointer should not be null!");

			// Create the physical device.
			setupPhysicalDevice(extensions, flag);

			// Create the logical device.
			setupLogicalDevice(extensions, flag);

			// Create the memory manager's allocator.
			createAllocator();
		}

		/**
		 * Virtual destructor.
		 */
		virtual ~Engine()
		{
			// Destroy the memory manager.
			destroyAllocator();

			// Destroy the logical device.
			vkDestroyDevice(m_vLogicalDevice, nullptr);
		}

		/**
		 * Get the logical device of the engine.
		 *
		 * @return The Vulkan logical device.
		 */
		VkDevice getLogicalDevice() const { return m_vLogicalDevice; }

		/**
		 * Get the physical device.
		 *
		 * @return The Vulkan physical device.
		 */
		VkPhysicalDevice getPhysicalDevice() const { return m_vPhysicalDevice; }

		/**
		 * Get the device table containing all the functions.
		 *
		 * @return The device table.
		 */
		VolkDeviceTable getDeviceTable() const { return m_DeviceTable; }

		/**
		 * Get the allocator.
		 *
		 * @return The allocator.
		 */
		VmaAllocator getAllocator() const { return m_vAllocator; }

		/**
		 * Get all the queue.
		 *
		 * @return The queues.
		 */
		std::vector<Queue> getQueues() const { return m_Queues; }

		/**
		 * Get a queue from the device.
		 * If the queue is not present, it'll throw an exception.
		 *
		 * @param flag The queue flag.
		 * @return The queue.
		 */
		Queue getQueue(const VkQueueFlagBits flag) const
		{
			for (const auto& queue : m_Queues)
				if (queue.getFlags() == flag)
					return queue;

			throw BackendError("Queue not found!");
		}

	private:
		/**
		 * Setup the physical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupPhysicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags)
		{
			// Get the Vulkan instance.
			const auto vInstance = m_pInstance->getInstance();

			// Enumerate physical devices.
			uint32_t deviceCount = 0;
			Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, nullptr), "Failed to enumerate physical devices.");

			// Throw an error if there are no physical devices available.
			if (deviceCount == 0)
				throw BackendError("No physical devices found!");

			std::vector<VkPhysicalDevice> vCandidates(deviceCount);
			Utility::ValidateResult(vkEnumeratePhysicalDevices(vInstance, &deviceCount, vCandidates.data()), "Failed to enumerate physical devices.");

			std::array<VkPhysicalDevice, 6> vPriorityMap;
			vPriorityMap.fill(VK_NULL_HANDLE);

			// Iterate through all the candidate devices and find the best device.
			for (const auto& vCandidateDevice : vCandidates)
			{
				// Check if the device is suitable for our use.
				if (isPhysicalDeviceSuitable(vCandidateDevice, extensions, flags))
				{
					VkPhysicalDeviceProperties vPhysicalDeviceProperties = {};
					vkGetPhysicalDeviceProperties(vCandidateDevice, &vPhysicalDeviceProperties);

					// Sort the candidates by priority.
					switch (vPhysicalDeviceProperties.deviceType)
					{
					case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
						vPriorityMap[0] = vCandidateDevice;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
						vPriorityMap[1] = vCandidateDevice;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
						vPriorityMap[2] = vCandidateDevice;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_CPU:
						vPriorityMap[3] = vCandidateDevice;
						break;

					case VK_PHYSICAL_DEVICE_TYPE_OTHER:
						vPriorityMap[4] = vCandidateDevice;
						break;

					default:
						vPriorityMap[5] = vCandidateDevice;
						break;
					}
				}
			}

			// Choose the physical device with the highest priority.
			for (const auto vCandidate : vPriorityMap)
			{
				if (vCandidate != VK_NULL_HANDLE)
				{
					m_vPhysicalDevice = vCandidate;
					break;
				}
			}

			// Validate if a physical device was found.
			if (m_vPhysicalDevice == VK_NULL_HANDLE)
				throw BackendError("Unable to find suitable physical device!");
		}

		/**
		 * Setup the logical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupLogicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags)
		{
			// Initialize the queue families.
			std::map<uint32_t, uint32_t> uniqueQueueFamilies;

			// Get the transfer queue if required.
			if (flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
			{
				const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
				m_Queues.emplace_back(queue);

				uniqueQueueFamilies[queue.getFamily().value()]++;
			}

			// Get the graphics queue if required.
			if (flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
			{
				const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
				m_Queues.emplace_back(queue);

				uniqueQueueFamilies[queue.getFamily().value()]++;
			}

			// Get the compute queue if required.
			if (flags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
			{
				const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
				m_Queues.emplace_back(queue);

				uniqueQueueFamilies[queue.getFamily().value()]++;
			}

			// Get the encode queue if required.
			if (flags & VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
			{
				const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
				m_Queues.emplace_back(queue);

				uniqueQueueFamilies[queue.getFamily().value()]++;
			}

			// else, get the decode queue if required.
			else if (flags & VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
			{
				const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR);
				m_Queues.emplace_back(queue);

				uniqueQueueFamilies[queue.getFamily().value()]++;
			}

			// Setup device queues.
			constexpr std::array<float, 4> priority = { 1.0f, 1.0f, 1.0f, 1.0f };
			std::vector<VkDeviceQueueCreateInfo> vQueueCreateInfos;
			vQueueCreateInfos.reserve(uniqueQueueFamilies.size());

			VkDeviceQueueCreateInfo vQueueCreateInfo = {};
			vQueueCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			vQueueCreateInfo.pQueuePriorities = priority.data();

			for (const auto& [family, count] : uniqueQueueFamilies)
			{
				vQueueCreateInfo.queueCount = count;
				vQueueCreateInfo.queueFamilyIndex = family;
				vQueueCreateInfos.emplace_back(vQueueCreateInfo);
			}

			// Physical device features.
			VkPhysicalDeviceFeatures vFeatures = {};

			// Device create info.
			VkDeviceCreateInfo vDeviceCreateInfo = {};
			vDeviceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			vDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vQueueCreateInfos.size());
			vDeviceCreateInfo.pQueueCreateInfos = vQueueCreateInfos.data();
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

			// Load the device table.
			volkLoadDeviceTable(&m_DeviceTable, m_vLogicalDevice);

			// Setup queues.
			for (auto& queue : m_Queues)
				getDeviceTable().vkGetDeviceQueue(m_vLogicalDevice, queue.getFamily().value(), 0, queue.getQueueAddr());
		}

		/**
		 * Check if the device supports the required extensions.
		 *
		 * @param vPhysicalDevice The physical device to check.
		 * @param deviceExtensions The device extensions to check.
		 * @return Boolean stating if its supported or not.
		 */
		bool checkDeviceExtensionSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions) const
		{
			// Get the extension count.
			uint32_t extensionCount = 0;
			Utility::ValidateResult(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, nullptr), "Failed to enumerate physical device extension property count!");

			// Load the extensions.
			std::vector<VkExtensionProperties> availableExtensions(extensionCount);
			Utility::ValidateResult(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, availableExtensions.data()), "Failed to enumerate physical device extension properties!");

			std::set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

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
		bool isPhysicalDeviceSuitable(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions, const VkQueueFlags flags) const
		{
			// Check if all the provided queue flags are supported.
			for (uint32_t i = 1; i < 0x00000080; i = i << 1)
			{
				if (i & flags && !Queue(vPhysicalDevice, static_cast<VkQueueFlagBits>(i)).isComplete())
					return false;
			}

			return checkDeviceExtensionSupport(vPhysicalDevice, deviceExtensions);
		}

		/**
		 * Get all the functions needed by VMA.
		 *
		 * @return The functions.
		 */
		VmaVulkanFunctions getVulkanFunctions() const
		{
			VmaVulkanFunctions functions = {};
			functions.vkAllocateMemory = getDeviceTable().vkAllocateMemory;
			functions.vkBindBufferMemory = getDeviceTable().vkBindBufferMemory;
			functions.vkBindBufferMemory2KHR = getDeviceTable().vkBindBufferMemory2;
			functions.vkBindImageMemory = getDeviceTable().vkBindImageMemory;
			functions.vkBindImageMemory2KHR = getDeviceTable().vkBindImageMemory2;
			functions.vkCmdCopyBuffer = getDeviceTable().vkCmdCopyBuffer;
			functions.vkCreateBuffer = getDeviceTable().vkCreateBuffer;
			functions.vkCreateImage = getDeviceTable().vkCreateImage;
			functions.vkDestroyBuffer = getDeviceTable().vkDestroyBuffer;
			functions.vkDestroyImage = getDeviceTable().vkDestroyImage;
			functions.vkFlushMappedMemoryRanges = getDeviceTable().vkFlushMappedMemoryRanges;
			functions.vkFreeMemory = getDeviceTable().vkFreeMemory;
			functions.vkGetBufferMemoryRequirements = getDeviceTable().vkGetBufferMemoryRequirements;
			functions.vkGetBufferMemoryRequirements2KHR = getDeviceTable().vkGetBufferMemoryRequirements2;
			functions.vkGetImageMemoryRequirements = getDeviceTable().vkGetImageMemoryRequirements;
			functions.vkGetImageMemoryRequirements2KHR = getDeviceTable().vkGetImageMemoryRequirements2;
			functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
			functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
			functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
			functions.vkInvalidateMappedMemoryRanges = getDeviceTable().vkInvalidateMappedMemoryRanges;
			functions.vkMapMemory = getDeviceTable().vkMapMemory;
			functions.vkUnmapMemory = getDeviceTable().vkUnmapMemory;

			return functions;
		}

		/**
		 * Create the VMA Allocator.
		 */
		void createAllocator()
		{
			VmaAllocatorCreateInfo vmaCreateInfo = {};
			vmaCreateInfo.instance = m_pInstance->getInstance();
			vmaCreateInfo.physicalDevice = getPhysicalDevice();
			vmaCreateInfo.device = getLogicalDevice();
			vmaCreateInfo.vulkanApiVersion = m_pInstance->getVulkanVersion();

			const VmaVulkanFunctions functions = getVulkanFunctions();
			vmaCreateInfo.pVulkanFunctions = &functions;

			Utility::ValidateResult(vmaCreateAllocator(&vmaCreateInfo, &m_vAllocator), "Failed to create the allocator!");
		}

		/**
		 * Destroy the VMA Allocator.
		 */
		void destroyAllocator()
		{
			vmaDestroyAllocator(m_vAllocator);
		}

	protected:
		std::shared_ptr<Instance> m_pInstance = nullptr;

		std::vector<Queue> m_Queues;

		VkDevice m_vLogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_vPhysicalDevice = VK_NULL_HANDLE;

	private:
		VolkDeviceTable m_DeviceTable;
		VmaAllocator m_vAllocator;
	};
}