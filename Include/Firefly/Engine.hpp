#pragma once

#include "Instance.hpp"

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
		 * @param flags The queue flag bits.
		 * @param extensions The device extensions to activate.
		 * @throws std::runtime_error If the pointer is null. It could also throw this same exception if there are no physical devices.
		 */
		explicit Engine(const std::shared_ptr<Instance>& pInstance, VkQueueFlags flags, const std::vector<const char*>& extensions, const VkPhysicalDeviceFeatures& features = VkPhysicalDeviceFeatures());

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
		 * Destroy the VMA Allocator.
		 */
		void destroyAllocator();

		/**
		 * Destroy a created command pool.
		 */
		void destroyCommandPool();

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