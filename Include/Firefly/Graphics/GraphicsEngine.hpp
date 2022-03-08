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
		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		GraphicsEngine(const std::shared_ptr<Instance>& pInstance)
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_TRANSFER_BIT | VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, { VK_KHR_SWAPCHAIN_EXTENSION_NAME })
		{
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
	};
}