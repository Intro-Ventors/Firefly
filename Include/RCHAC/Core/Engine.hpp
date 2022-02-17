#pragma once

#include "Utility.hpp"
#include <vk_video/vulkan_video_codec_h265std.h>
#include <memory>

namespace RCHAC
{
	class Instance;

	/**
	 * RCHAC Engine class.
	 * This class is the base class for the two engines, Encoder and Decoder.
	 */
	class Engine
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound to.
		 * @param flag The queue flag bits.
		 * @throws std::runtime_error If the pointer is null. It could also throw this same exception if there are no physical devices.
		 */
		Engine(const std::shared_ptr<Instance>& pInstance, const VkQueueFlagBits flag);

		/**
		 * Virtual destructor.
		 */
		virtual ~Engine();

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

	protected:
		std::shared_ptr<Instance> m_pInstance = nullptr;

		VkDevice m_vLogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_vPhysicalDevice = VK_NULL_HANDLE;
	};
}