#include "Firefly/Graphics/GraphicsEngine.hpp"

namespace Firefly
{
	GraphicsEngine::GraphicsEngine(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, {}, getFeatures())
	{
	}

	std::shared_ptr<GraphicsEngine> GraphicsEngine::create(const std::shared_ptr<Instance>& pInstance)
	{
		return std::make_shared<GraphicsEngine>(pInstance);
	}

	VkPhysicalDeviceFeatures GraphicsEngine::getFeatures() const
	{
		VkPhysicalDeviceFeatures vFeatures = {};
		vFeatures.samplerAnisotropy = VK_TRUE;
		vFeatures.sampleRateShading = VK_TRUE;

		return vFeatures;
	}
}