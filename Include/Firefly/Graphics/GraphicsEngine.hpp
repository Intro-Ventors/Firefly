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
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, { VK_KHR_SWAPCHAIN_EXTENSION_NAME }, getFeatures())
		{

		}

		/**
		 * Destructor.
		 */
		~GraphicsEngine()
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

	private:
		/**
		 * Get all the required features.
		 *
		 * @return The physical device features.
		 */
		VkPhysicalDeviceFeatures getFeatures() const
		{
			VkPhysicalDeviceFeatures vFeatures = {};
			vFeatures.samplerAnisotropy = VK_TRUE;

			return vFeatures;
		}
	};
}