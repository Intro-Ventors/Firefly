#pragma once

#include "Firefly/Engine.hpp"

namespace Firefly
{
	/**
	 * Firefly Encoder class.
	 * This class is the main Encoder engine.
	 */
	class GraphicsEngine final : public Engine
	{
	public:
		FIREFLY_DEFAULT_COPY(GraphicsEngine);
		FIREFLY_DEFAULT_MOVE(GraphicsEngine);

		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		explicit GraphicsEngine(const std::shared_ptr<Instance>& pInstance)
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT | VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, { VK_KHR_SWAPCHAIN_EXTENSION_NAME })
		{
			// Create the command pool.
			createCommandPool();
		}

		/**
		 * Destructor.
		 */
		~GraphicsEngine()
		{
			// Destroy the command pool.
			destroyCommandPool();
		}

		/**
		 * Create a new graphics engine.
		 *
		 * @param pInstance The instance pointer.
		 * @rerurn The created engine pointer.
		 */
		static std::shared_ptr<GraphicsEngine> create(const std::shared_ptr<Instance>& pInstance)
		{
			return std::make_shared<GraphicsEngine>(pInstance);
		}

	private:
		/**
		 * Create the command pool.
		 */
		void createCommandPool()
		{
			const auto queue = getQueue(VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT);

			VkCommandPoolCreateInfo vCreateInfo = {};
			vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			vCreateInfo.flags = 0;
			vCreateInfo.pNext = VK_NULL_HANDLE;
			vCreateInfo.queueFamilyIndex = queue.getFamily().value();

			Utility::ValidateResult(getDeviceTable().vkCreateCommandPool(getLogicalDevice(), &vCreateInfo, nullptr, &m_vCommandPool), "Failed to create the command pool!");
		}

		/**
		 * Destroy a created command pool.
		 */
		void destroyCommandPool()
		{
			getDeviceTable().vkDestroyCommandPool(getLogicalDevice(), m_vCommandPool, nullptr);
		}

	private:
		VkCommandPool m_vCommandPool = VK_NULL_HANDLE;
		VkCommandBuffer m_vCommandBuffer = VK_NULL_HANDLE;
	};
}