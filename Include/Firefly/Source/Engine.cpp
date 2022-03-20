#include "Firefly/Engine.hpp"

#include <set>
#include <map>
#include <array>

#ifdef max
#undef max

#endif

namespace /* anonymous */
{
	VkPhysicalDeviceFeatures ResolvePhysicalDeviceFeatures(const VkPhysicalDevice vPhysicalDevice, const VkPhysicalDeviceFeatures& features)
	{
		VkPhysicalDeviceFeatures vAvailableFeatures = {};
		vkGetPhysicalDeviceFeatures(vPhysicalDevice, &vAvailableFeatures);

		vAvailableFeatures.robustBufferAccess &= features.robustBufferAccess;
		vAvailableFeatures.fullDrawIndexUint32 &= features.fullDrawIndexUint32;
		vAvailableFeatures.imageCubeArray &= features.imageCubeArray;
		vAvailableFeatures.independentBlend &= features.independentBlend;
		vAvailableFeatures.geometryShader &= features.geometryShader;
		vAvailableFeatures.tessellationShader &= features.tessellationShader;
		vAvailableFeatures.sampleRateShading &= features.sampleRateShading;
		vAvailableFeatures.dualSrcBlend &= features.dualSrcBlend;
		vAvailableFeatures.logicOp &= features.logicOp;
		vAvailableFeatures.multiDrawIndirect &= features.multiDrawIndirect;
		vAvailableFeatures.drawIndirectFirstInstance &= features.drawIndirectFirstInstance;
		vAvailableFeatures.depthClamp &= features.depthClamp;
		vAvailableFeatures.depthBiasClamp &= features.depthBiasClamp;
		vAvailableFeatures.fillModeNonSolid &= features.fillModeNonSolid;
		vAvailableFeatures.depthBounds &= features.depthBounds;
		vAvailableFeatures.wideLines &= features.wideLines;
		vAvailableFeatures.largePoints &= features.largePoints;
		vAvailableFeatures.alphaToOne &= features.alphaToOne;
		vAvailableFeatures.multiViewport &= features.multiViewport;
		vAvailableFeatures.samplerAnisotropy &= features.samplerAnisotropy;
		vAvailableFeatures.textureCompressionETC2 &= features.textureCompressionETC2;
		vAvailableFeatures.textureCompressionASTC_LDR &= features.textureCompressionASTC_LDR;
		vAvailableFeatures.textureCompressionBC &= features.textureCompressionBC;
		vAvailableFeatures.occlusionQueryPrecise &= features.occlusionQueryPrecise;
		vAvailableFeatures.pipelineStatisticsQuery &= features.pipelineStatisticsQuery;
		vAvailableFeatures.vertexPipelineStoresAndAtomics &= features.vertexPipelineStoresAndAtomics;
		vAvailableFeatures.fragmentStoresAndAtomics &= features.fragmentStoresAndAtomics;
		vAvailableFeatures.shaderTessellationAndGeometryPointSize &= features.shaderTessellationAndGeometryPointSize;
		vAvailableFeatures.shaderImageGatherExtended &= features.shaderImageGatherExtended;
		vAvailableFeatures.shaderStorageImageExtendedFormats &= features.shaderStorageImageExtendedFormats;
		vAvailableFeatures.shaderStorageImageMultisample &= features.shaderStorageImageMultisample;
		vAvailableFeatures.shaderStorageImageReadWithoutFormat &= features.shaderStorageImageReadWithoutFormat;
		vAvailableFeatures.shaderStorageImageWriteWithoutFormat &= features.shaderStorageImageWriteWithoutFormat;
		vAvailableFeatures.shaderUniformBufferArrayDynamicIndexing &= features.shaderUniformBufferArrayDynamicIndexing;
		vAvailableFeatures.shaderSampledImageArrayDynamicIndexing &= features.shaderSampledImageArrayDynamicIndexing;
		vAvailableFeatures.shaderStorageBufferArrayDynamicIndexing &= features.shaderStorageBufferArrayDynamicIndexing;
		vAvailableFeatures.shaderStorageImageArrayDynamicIndexing &= features.shaderStorageImageArrayDynamicIndexing;
		vAvailableFeatures.shaderClipDistance &= features.shaderClipDistance;
		vAvailableFeatures.shaderCullDistance &= features.shaderCullDistance;
		vAvailableFeatures.shaderFloat64 &= features.shaderFloat64;
		vAvailableFeatures.shaderInt64 &= features.shaderInt64;
		vAvailableFeatures.shaderInt16 &= features.shaderInt16;
		vAvailableFeatures.shaderResourceResidency &= features.shaderResourceResidency;
		vAvailableFeatures.shaderResourceMinLod &= features.shaderResourceMinLod;
		vAvailableFeatures.sparseBinding &= features.sparseBinding;
		vAvailableFeatures.sparseResidencyBuffer &= features.sparseResidencyBuffer;
		vAvailableFeatures.sparseResidencyImage2D &= features.sparseResidencyImage2D;
		vAvailableFeatures.sparseResidencyImage3D &= features.sparseResidencyImage3D;
		vAvailableFeatures.sparseResidency2Samples &= features.sparseResidency2Samples;
		vAvailableFeatures.sparseResidency4Samples &= features.sparseResidency4Samples;
		vAvailableFeatures.sparseResidency8Samples &= features.sparseResidency8Samples;
		vAvailableFeatures.sparseResidency16Samples &= features.sparseResidency16Samples;
		vAvailableFeatures.sparseResidencyAliased &= features.sparseResidencyAliased;
		vAvailableFeatures.variableMultisampleRate &= features.variableMultisampleRate;
		vAvailableFeatures.inheritedQueries &= features.inheritedQueries;

		return vAvailableFeatures;
	}

	bool CheckDeviceExtensionSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions)
	{
		// Get the extension count.
		uint32_t extensionCount = 0;
		FIREFLY_VALIDATE(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, nullptr), "Failed to enumerate physical device extension property count!");

		// Load the extensions.
		std::vector<VkExtensionProperties> availableExtensions(extensionCount);
		FIREFLY_VALIDATE(vkEnumerateDeviceExtensionProperties(vPhysicalDevice, nullptr, &extensionCount, availableExtensions.data()), "Failed to enumerate physical device extension properties!");

		std::set<std::string_view> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

		// Iterate and check if it contains the extensions we need. If it does, remove them from the set so we can later check if 
		// all the required extensions exist.
		for (const VkExtensionProperties& extension : availableExtensions)
			requiredExtensions.erase(extension.extensionName);

		// If the required extensions set is empty, it means that all the required extensions exist within the physical device.
		return requiredExtensions.empty();
	}

	bool IsPhysicalDeviceSuitable(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions, const VkQueueFlags flags)
	{
		// Check if all the provided queue flags are supported.
		for (uint32_t i = 1; i < flags; i = i << 1)
		{
			if (i & flags && !Firefly::Queue(vPhysicalDevice, static_cast<VkQueueFlagBits>(i)).isComplete())
				return false;
		}

		return CheckDeviceExtensionSupport(vPhysicalDevice, deviceExtensions);
	}

	Firefly::Queue FindQueue(const std::vector<Firefly::Queue>& queues, const VkQueueFlagBits flag)
	{
		for (const auto& queue : queues)
			if (queue.getFlags() == flag)
				return queue;

		throw Firefly::BackendError("Queue not found!");
	}
}

namespace Firefly
{
	Engine::Engine(const std::shared_ptr<Instance>& pInstance, VkQueueFlags flags, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features)
		: m_pInstance(pInstance)
	{
		// Validate the pointer.
		if (!m_pInstance)
			throw BackendError("The instance pointer should not be null!");

		// Make sure that we have the transfer queue.
		flags |= VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT;

		// Create the physical device.
		// Get the Vulkan instance.
		const auto vInstance = m_pInstance->getInstance();

		// Enumerate physical devices.
		uint32_t deviceCount = 0;
		FIREFLY_VALIDATE(vkEnumeratePhysicalDevices(vInstance, &deviceCount, nullptr), "Failed to enumerate physical devices.");

		// Throw an error if there are no physical devices available.
		if (deviceCount == 0)
			throw BackendError("No physical devices found!");

		std::vector<VkPhysicalDevice> vCandidates(deviceCount);
		FIREFLY_VALIDATE(vkEnumeratePhysicalDevices(vInstance, &deviceCount, vCandidates.data()), "Failed to enumerate physical devices.");

		struct Candidate { VkPhysicalDeviceProperties m_Properties; VkPhysicalDevice m_Candidate; };
		std::array<Candidate, 6> vPriorityMap;

		// Iterate through all the candidate devices and find the best device.
		for (const auto& vCandidateDevice : vCandidates)
		{
			// Check if the device is suitable for our use.
			if (IsPhysicalDeviceSuitable(vCandidateDevice, extensions, flags))
			{
				VkPhysicalDeviceProperties vPhysicalDeviceProperties = {};
				vkGetPhysicalDeviceProperties(vCandidateDevice, &vPhysicalDeviceProperties);

				// Sort the candidates by priority.
				uint8_t priorityIndex = 5;
				switch (vPhysicalDeviceProperties.deviceType)
				{
				case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
					priorityIndex = 0;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
					priorityIndex = 1;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
					priorityIndex = 2;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_CPU:
					priorityIndex = 3;
					break;

				case VK_PHYSICAL_DEVICE_TYPE_OTHER:
					priorityIndex = 4;
					break;

				default:
					priorityIndex = 5;
					break;
				}

				vPriorityMap[priorityIndex].m_Candidate = vCandidateDevice;
				vPriorityMap[priorityIndex].m_Properties = vPhysicalDeviceProperties;
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

		// If we found a suitable physical device, lets log it.
		FIREFLY_LOG_INFO("Physical device found.");

		// Create the logical device.
		// Initialize the queue families.
		std::map<uint32_t, uint32_t> uniqueQueueFamilies;

		// Get the transfer queue if required.
		if (flags & VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT)
		{
			const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);
			FIREFLY_LOG_INFO("Created the transfer queue.");

			m_Queues.emplace_back(queue);
			uniqueQueueFamilies[queue.getFamily().value()]++;
		}

		// Get the graphics queue if required.
		if (flags & VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT)
		{
			const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT);
			FIREFLY_LOG_INFO("Created the graphics queue.");

			m_Queues.emplace_back(queue);
			uniqueQueueFamilies[queue.getFamily().value()]++;
		}

		// Get the compute queue if required.
		if (flags & VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT)
		{
			const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_COMPUTE_BIT);
			FIREFLY_LOG_INFO("Created the compute queue.");
			m_Queues.emplace_back(queue);

			uniqueQueueFamilies[queue.getFamily().value()]++;
		}

		// Get the encode queue if required.
		if (flags & VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
		{
			const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR);
			FIREFLY_LOG_INFO("Created the encode queue.");
			m_Queues.emplace_back(queue);

			uniqueQueueFamilies[queue.getFamily().value()]++;
		}

		// else, get the decode queue if required.
		else if (flags & VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
		{
			const auto queue = Queue(m_vPhysicalDevice, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR);
			FIREFLY_LOG_INFO("Created the decode queue.");

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

		const auto vRequiredFeatures = ResolvePhysicalDeviceFeatures(m_vPhysicalDevice, features);

		// Device create info.
		VkDeviceCreateInfo vDeviceCreateInfo = {};
		vDeviceCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		vDeviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(vQueueCreateInfos.size());
		vDeviceCreateInfo.pQueueCreateInfos = vQueueCreateInfos.data();
		vDeviceCreateInfo.pEnabledFeatures = &vRequiredFeatures;
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
		FIREFLY_VALIDATE(vkCreateDevice(m_vPhysicalDevice, &vDeviceCreateInfo, nullptr, &m_vLogicalDevice), "Failed to create the logical device!");

		// Load the device table.
		volkLoadDeviceTable(&m_DeviceTable, m_vLogicalDevice);

		// Setup queues.
		for (auto& queue : m_Queues)
			m_DeviceTable.vkGetDeviceQueue(m_vLogicalDevice, queue.getFamily().value(), 0, queue.getQueueAddr());

		// Create the memory manager's allocator.
		VmaAllocatorCreateInfo vmaCreateInfo = {};
		vmaCreateInfo.instance = m_pInstance->getInstance();
		vmaCreateInfo.physicalDevice = m_vPhysicalDevice;
		vmaCreateInfo.device = m_vLogicalDevice;
		vmaCreateInfo.vulkanApiVersion = m_pInstance->getVulkanVersion();

		VmaVulkanFunctions functions = {};
		functions.vkGetInstanceProcAddr = vkGetInstanceProcAddr;
		functions.vkGetDeviceProcAddr = vkGetDeviceProcAddr;

		functions.vkAllocateMemory = m_DeviceTable.vkAllocateMemory;
		functions.vkBindBufferMemory = m_DeviceTable.vkBindBufferMemory;
		functions.vkBindBufferMemory2KHR = m_DeviceTable.vkBindBufferMemory2;
		functions.vkBindImageMemory = m_DeviceTable.vkBindImageMemory;
		functions.vkBindImageMemory2KHR = m_DeviceTable.vkBindImageMemory2;
		functions.vkCmdCopyBuffer = m_DeviceTable.vkCmdCopyBuffer;
		functions.vkCreateBuffer = m_DeviceTable.vkCreateBuffer;
		functions.vkCreateImage = m_DeviceTable.vkCreateImage;
		functions.vkDestroyBuffer = m_DeviceTable.vkDestroyBuffer;
		functions.vkDestroyImage = m_DeviceTable.vkDestroyImage;
		functions.vkFlushMappedMemoryRanges = m_DeviceTable.vkFlushMappedMemoryRanges;
		functions.vkFreeMemory = m_DeviceTable.vkFreeMemory;
		functions.vkGetBufferMemoryRequirements = m_DeviceTable.vkGetBufferMemoryRequirements;
		functions.vkGetBufferMemoryRequirements2KHR = m_DeviceTable.vkGetBufferMemoryRequirements2;
		functions.vkGetImageMemoryRequirements = m_DeviceTable.vkGetImageMemoryRequirements;
		functions.vkGetImageMemoryRequirements2KHR = m_DeviceTable.vkGetImageMemoryRequirements2;
		functions.vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties;
		functions.vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2;
		functions.vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties;
		functions.vkInvalidateMappedMemoryRanges = m_DeviceTable.vkInvalidateMappedMemoryRanges;
		functions.vkMapMemory = m_DeviceTable.vkMapMemory;
		functions.vkUnmapMemory = m_DeviceTable.vkUnmapMemory;

		vmaCreateInfo.pVulkanFunctions = &functions;

		FIREFLY_VALIDATE(vmaCreateAllocator(&vmaCreateInfo, &m_vAllocator), "Failed to create the allocator!");

		// Create the command pool.
		const auto queue = FindQueue(m_Queues, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);

		VkCommandPoolCreateInfo vCommandPoolCreateInfo = {};
		vCommandPoolCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		vCommandPoolCreateInfo.flags = VkCommandPoolCreateFlagBits::VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		vCommandPoolCreateInfo.pNext = VK_NULL_HANDLE;
		vCommandPoolCreateInfo.queueFamilyIndex = queue.getFamily().value();

		FIREFLY_VALIDATE(m_DeviceTable.vkCreateCommandPool(m_vLogicalDevice, &vCommandPoolCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");

		// Allocate the command buffer.
		VkCommandBufferAllocateInfo vAllocateInfo = {};
		vAllocateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		vAllocateInfo.pNext = VK_NULL_HANDLE;
		vAllocateInfo.level = VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		vAllocateInfo.commandPool = m_vCommandPool;
		vAllocateInfo.commandBufferCount = 1;

		FIREFLY_VALIDATE(m_DeviceTable.vkAllocateCommandBuffers(m_vLogicalDevice, &vAllocateInfo, &m_vCommandBuffer), "Failed to allocate command buffer!");
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

		FIREFLY_VALIDATE(m_DeviceTable.vkBeginCommandBuffer(m_vCommandBuffer, &vBeginInfo), "Failed to begin command buffer recording!");

		m_bIsCommandBufferRecording = true;
		return m_vCommandBuffer;
	}

	void Engine::endCommandBufferRecording()
	{
		// Skip if we weren't recording.
		if (!m_bIsCommandBufferRecording)
			return;

		FIREFLY_VALIDATE(m_DeviceTable.vkEndCommandBuffer(m_vCommandBuffer), "Failed to end command buffer recording!");

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

			FIREFLY_VALIDATE(m_DeviceTable.vkCreateFence(getLogicalDevice(), &vFenceCreateInfo, nullptr, &vFence), "Failed to create the synchronization fence!");
		}

		// Submit the queue.
		FIREFLY_VALIDATE(m_DeviceTable.vkQueueSubmit(queue.getQueue(), 1, &vSubmitInfo, vFence), "Failed to submit the queue!");

		// Destroy the fence if we created it.
		if (shouldWait)
		{
			FIREFLY_VALIDATE(m_DeviceTable.vkWaitForFences(getLogicalDevice(), 1, &vFence, VK_TRUE, std::numeric_limits<uint64_t>::max()), "Failed to wait for the fence!");
			m_DeviceTable.vkDestroyFence(getLogicalDevice(), vFence, nullptr);
		}
	}

	Queue Engine::getQueue(const VkQueueFlagBits flag) const
	{
		return FindQueue(m_Queues, flag);
	}

	VkFormat Engine::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const
	{
		for (const auto format : candidates) 
		{
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

	void Engine::destroyAllocator()
	{
		vmaDestroyAllocator(m_vAllocator);
	}

	void Engine::destroyCommandPool()
	{
		m_DeviceTable.vkDestroyCommandPool(getLogicalDevice(), m_vCommandPool, nullptr);
	}

	void Engine::freeCommandBuffer()
	{
		m_DeviceTable.vkFreeCommandBuffers(getLogicalDevice(), m_vCommandPool, 1, &m_vCommandBuffer);
	}
}