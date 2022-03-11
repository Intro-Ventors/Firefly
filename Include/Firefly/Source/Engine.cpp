#include "Firefly/Engine.hpp"

namespace Firefly
{
	Engine::Engine(const std::shared_ptr<Instance>& pInstance, VkQueueFlags flag, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features)
		: m_pInstance(pInstance)
	{
		// Validate the pointer.
		if (!m_pInstance)
			throw BackendError("The instance pointer should not be null!");

		// Make sure that we have the transfer queue.
		flag |= VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT;

		// Create the physical device.
		setupPhysicalDevice(extensions, flag);

		// Create the logical device.
		setupLogicalDevice(extensions, flag, features);

		// Create the memory manager's allocator.
		createAllocator();

		// Create the command pool.
		createCommandPool();

		// Allocate the command buffer.
		allocateCommandBuffer();
	}
	
	Engine::~Engine()
	{
		// Destroy the memory manager.
		destroyAllocator();

		// Free the command buffers.
		freeCommandBuffer();

		// Destroy the command pool.
		destroyCommandPool();

		// Destroy the logical device.
		vkDestroyDevice(m_vLogicalDevice, nullptr);
	}
	
	VkCommandBuffer Engine::beginCommandBufferRecording()
	{
		// Skip if we're on the recording state.
		if (m_bIsCommandBufferRecording)
			return m_vCommandBuffer;

		// Begin recording.
		VkCommandBufferBeginInfo vBeginInfo = {};
		vBeginInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		vBeginInfo.pNext = nullptr;
		vBeginInfo.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		Utility::ValidateResult(getDeviceTable().vkBeginCommandBuffer(m_vCommandBuffer, &vBeginInfo), "Failed to begin command buffer recording!");

		m_bIsCommandBufferRecording = true;
		return m_vCommandBuffer;
	}
	
	void Engine::endCommandBufferRecording()
	{
		// Skip if we weren't recording.
		if (!m_bIsCommandBufferRecording)
			return;

		Utility::ValidateResult(getDeviceTable().vkEndCommandBuffer(m_vCommandBuffer), "Failed to end command buffer recording!");

		m_bIsCommandBufferRecording = false;
	}
	
	void Engine::executeRecordedCommands(bool shouldWait)
	{
		// End recording if we haven't.
		endCommandBufferRecording();

		const auto queue = getQueue(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);

		VkSubmitInfo vSubmitInfo = {};
		vSubmitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO;
		vSubmitInfo.commandBufferCount = 1;
		vSubmitInfo.pCommandBuffers = &m_vCommandBuffer;

		VkFence vFence = VK_NULL_HANDLE;

		// Create the fence if we need to wait.
		if (shouldWait)
		{
			VkFenceCreateInfo vFenceCreateInfo = {};
			vFenceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
			vFenceCreateInfo.pNext = VK_NULL_HANDLE;
			vFenceCreateInfo.flags = 0;

			Utility::ValidateResult(getDeviceTable().vkCreateFence(getLogicalDevice(), &vFenceCreateInfo, nullptr, &vFence), "Failed to create the synchronization fence!");
		}

		// Submit the queue.
		Utility::ValidateResult(getDeviceTable().vkQueueSubmit(queue.getQueue(), 1, &vSubmitInfo, vFence), "Failed to submit the queue!");

		// Destroy the fence if we created it.
		if (shouldWait)
		{
			Utility::ValidateResult(getDeviceTable().vkWaitForFences(getLogicalDevice(), 1, &vFence, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait for the fence!");
			getDeviceTable().vkDestroyFence(getLogicalDevice(), vFence, nullptr);
		}
	}
	
	Queue Engine::getQueue(const VkQueueFlagBits flag) const
	{
		for (const auto& queue : m_Queues)
			if (queue.getFlags() == flag)
				return queue;

		throw BackendError("Queue not found!");
	}
	
	VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		for (const auto format : candidates) {
			VkFormatProperties props;
			vkGetPhysicalDeviceFormatProperties(getPhysicalDevice(), format, &props);

			if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
				return format;
			else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
				return format;
		}

		throw BackendError("Failed to find supported format!");
	}
	
	VkFormat Engine::findBestDepthFormat() const
	{
		return findSupportedFormat(
			{ VkFormat::VK_FORMAT_D32_SFLOAT_S8_UINT, VkFormat::VK_FORMAT_D24_UNORM_S8_UINT, VkFormat::VK_FORMAT_D32_SFLOAT },
			VkImageTiling::VK_IMAGE_TILING_OPTIMAL, VkFormatFeatureFlagBits::VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
	
	void Engine::setupPhysicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags)
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

		struct Candidate { VkPhysicalDeviceProperties m_Properties; VkPhysicalDevice m_Candidate; };
		std::array<Candidate, 6> vPriorityMap;

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
					vPriorityMap[0].m_Candidate = vCandidateDevice;
					vPriorityMap[0].m_Properties = vPhysicalDeviceProperties;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					vPriorityMap[1].m_Candidate = vCandidateDevice;
					vPriorityMap[1].m_Properties = vPhysicalDeviceProperties;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					vPriorityMap[2].m_Candidate = vCandidateDevice;
					vPriorityMap[2].m_Properties = vPhysicalDeviceProperties;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					vPriorityMap[3].m_Candidate = vCandidateDevice;
					vPriorityMap[3].m_Properties = vPhysicalDeviceProperties;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					vPriorityMap[4].m_Candidate = vCandidateDevice;
					vPriorityMap[4].m_Properties = vPhysicalDeviceProperties;
					break;

				default:
					vPriorityMap[5].m_Candidate = vCandidateDevice;
					vPriorityMap[5].m_Properties = vPhysicalDeviceProperties;
					break;
				}
			}
		}

		// Choose the physical device with the highest priority.
		for (const auto& candidate : vPriorityMap)
		{
			if (candidate.m_Candidate != VK_NULL_HANDLE)
			{
				m_vPhysicalDevice = candidate.m_Candidate;
				m_Properties = candidate.m_Properties;
				break;
			}
		}

		// Validate if a physical device was found.
		if (m_vPhysicalDevice == VK_NULL_HANDLE)
			throw BackendError("Unable to find suitable physical device!");
	}
	
	void Engine::setupLogicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags, const VkPhysicalDeviceFeatures& features)
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

		// Device create info.
		VkDeviceCreateInfo vDeviceCreateInfo = {};
		vDeviceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		vDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vQueueCreateInfos.size());
		vDeviceCreateInfo.pQueueCreateInfos = vQueueCreateInfos.data();
		vDeviceCreateInfo.pEnabledFeatures = &features;
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
	
	bool Engine::checkDeviceExtensionSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions) const
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
	
	bool Engine::isPhysicalDeviceSuitable(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions, const VkQueueFlags flags) const
	{
		// Check if all the provided queue flags are supported.
		for (uint32_t i = 1; i < 0x00000080; i = i << 1)
		{
			if (i & flags && !Queue(vPhysicalDevice, static_cast<VkQueueFlagBits>(i)).isComplete())
				return false;
		}

		return checkDeviceExtensionSupport(vPhysicalDevice, deviceExtensions);
	}
	
	VmaVulkanFunctions Engine::getVulkanFunctions() const
	{
		VmaVulkanFunctions functions = {};
		functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

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
	
	void Engine::createAllocator()
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
	
	void Engine::destroyAllocator()
	{
		vmaDestroyAllocator(m_vAllocator);
	}
	
	void Engine::createCommandPool()
	{
		const auto queue = getQueue(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);

		VkCommandPoolCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vCreateInfo.pNext = VK_NULL_HANDLE;
		vCreateInfo.queueFamilyIndex = queue.getFamily().value();

		Utility::ValidateResult(getDeviceTable().vkCreateCommandPool(getLogicalDevice(), &vCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");
	}
	
	void Engine::destroyCommandPool()
	{
		getDeviceTable().vkDestroyCommandPool(getLogicalDevice(), m_vCommandPool, nullptr);
	}
	
	void Engine::allocateCommandBuffer()
	{
		VkCommandBufferAllocateInfo vAllocateInfo = {};
		vAllocateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vAllocateInfo.pNext = VK_NULL_HANDLE;
		vAllocateInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vAllocateInfo.commandPool = m_vCommandPool;
		vAllocateInfo.commandBufferCount = 1;

		Utility::ValidateResult(getDeviceTable().vkAllocateCommandBuffers(getLogicalDevice(), &vAllocateInfo, &m_vCommandBuffer), "Failed to allocate command buffer!");
	}
	
	void Engine::freeCommandBuffer()
	{
		getDeviceTable().vkFreeCommandBuffers(getLogicalDevice(), m_vCommandPool, 1, &m_vCommandBuffer);
	}
}