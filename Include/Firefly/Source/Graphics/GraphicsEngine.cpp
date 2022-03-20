#include "Firefly/Graphics/GraphicsEngine.hpp"

namespace /* anonymous */
{
	constexpr VkPhysicalDeviceFeatures GetFeatures()
	{
		VkPhysicalDeviceFeatures vFeatures = {};
		vFeatures.samplerAnisotropy = VK_TRUE;
		vFeatures.sampleRateShading = VK_TRUE;
		vFeatures.tessellationShader = VK_TRUE;

		return vFeatures;
	}
}

namespace Firefly
{
	GraphicsEngine::GraphicsEngine(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, {}, GetFeatures())
	{
	}

	std::shared_ptr<GraphicsEngine> GraphicsEngine::create(const std::shared_ptr<Instance>& pInstance)
	{
		return std::make_shared<GraphicsEngine>(pInstance);
	}
}