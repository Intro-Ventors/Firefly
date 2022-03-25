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
		: Engine(pInstance)
	{
	}

	std::shared_ptr<GraphicsEngine> GraphicsEngine::create(const std::shared_ptr<Instance>& pInstance)
	{
		const auto pointer = std::make_shared<GraphicsEngine>(pInstance);
		FIREFLY_VALIDATE_OBJECT(pointer);

		pointer->initialize(VkQueueFlagBits::VK_QUEUE_GRAPHICS_BIT, {}, GetFeatures());

		return pointer;
	}
}