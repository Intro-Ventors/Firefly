#pragma once

#include "Queue.hpp"

#include <memory>
#include <vector>

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

	private:
		/**
		 * Setup the physical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupPhysicalDevice(const std::vector<const char*>& extensions, const VkQueueFlagBits flag);

		/**
		 * Setup the logical device.
		 *
		 * @param extensions The required extension.
		 * @param flag The queue flag bits.
		 */
		void setupLogicalDevice(const std::vector<const char*>& extensions, const VkQueueFlagBits flag);

	protected:
		std::shared_ptr<Instance> m_pInstance = nullptr;

		Queue m_Queue = {};

		VkDevice m_vLogicalDevice = VK_NULL_HANDLE;
		VkPhysicalDevice m_vPhysicalDevice = VK_NULL_HANDLE;
	};
}