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
		FIREFLY_NO_COPY(Engine);
		FIREFLY_DEFAULT_MOVE(Engine);

		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound to.
		 * @param flag The queue flag bits.
		 * @param extensions The device extensions to activate.
		 * @throws std::runtime_error If the pointer is null. It could also throw this same exception if there are no physical devices.
		 */
		explicit Engine(const std::shared_ptr<Instance>& pInstance, VkQueueFlags flag, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features = VkPhysicalDeviceFeatures());

		/**
		 * Virtual destructor.
		 */
		virtual ~Engine();

		/**
		 * Begin command buffer recording.
		 * If the command buffer is in the recording state, this will only return the command buffer.
		 *
		 * @return The command buffer.
		 */
		VkCommandBuffer beginCommandBufferRecording();

		/**
		 * End the command buffer recording.
		 */
		void endCommandBufferRecording();

		/**
		 * Execute the recorded commands.
		 *
		 * @param shouldWait Whether or not if we should wait until the GPU finishes execution. Default is true.
		 */
		void executeRecordedCommands(bool shouldWait = true);

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
		Queue getQueue(const VkQueueFlagBits flag) const;

		/**
		 * Get all the physical device properties.
		 *
		 * @return The properties.
		 */
		VkPhysicalDeviceProperties getPhysicalDeviceProperties() const { return m_Properties; }

		/**
		 * Find a supported format from a given list.
		 *
		 * @param candidates The candidate formats.
		 * @param tiling The image tiling.
		 * @param features The image tiling features.
		 * @return The supported format.
		 */
		VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) const;

		/**
		 * Find the best supported depth format.
		 *
		 * @return The depth format.
		 */
		VkFormat findBestDepthFormat() const;

	private:
		/**
		 * Setup the physical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupPhysicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags);

		/**
		 * Setup the logical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupLogicalDevice(const std::vector<const char*>& extensions, const VkQueueFlags flags, const VkPhysicalDeviceFeatures& features);

		/**
		 * Check if the device supports the required extensions.
		 *
		 * @param vPhysicalDevice The physical device to check.
		 * @param deviceExtensions The device extensions to check.
		 * @return Boolean stating if its supported or not.
		 */
		bool checkDeviceExtensionSupport(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions) const;

		/**
		 * Check if the physical device is suitable for use.
		 *
		 * @param vPhysicalDevice The physical device to check.
		 * @param deviceExtensions The device extensions to check with.
		 * @param flags The device flags that are needed.
		 * @return Boolean value stating if its viable or not.
		 */
		bool isPhysicalDeviceSuitable(VkPhysicalDevice vPhysicalDevice, const std::vector<const char*>& deviceExtensions, const VkQueueFlags flags) const;

		/**
		 * Get all the functions needed by VMA.
		 *
		 * @return The functions.
		 */
		VmaVulkanFunctions getVulkanFunctions() const;

		/**
		 * Create the VMA Allocator.
		 */
		void createAllocator();

		/**
		 * Destroy the VMA Allocator.
		 */
		void destroyAllocator();

		/**
		 * Create the command pool.
		 */
		void createCommandPool();

		/**
		 * Destroy a created command pool.
		 */
		void destroyCommandPool();

		/**
		 * Allocate the command buffer.
		 */
		void allocateCommandBuffer();

		/**
		 * Free the command buffer.
		 */
		void freeCommandBuffer();

	private:
		VkPhysicalDeviceProperties m_Properties = {};

		std::shared_ptr<Instance> m_pInstance = nullptr;

		std::vector<Queue> m_Queues;

		VkDevice m_vLogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_vPhysicalDevice = VK_NULL_HANDLE;

		VkCommandPool m_vCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_vCommandBuffer = VK_NULL_HANDLE;

		VolkDeviceTable m_DeviceTable;
		VmaAllocator m_vAllocator;

		bool m_bIsCommandBufferRecording = false;
	};
}