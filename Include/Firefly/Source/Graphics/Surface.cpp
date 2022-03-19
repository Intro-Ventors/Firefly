#include "Firefly/Graphics/Surface.hpp"

#include <GLFW/glfw3.h>

namespace Firefly
{
	Surface::Surface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height)
		: m_pInstance(pInstance), m_Width(width), m_Height(height)
	{
		if (!pInstance)
			throw BackendError("The instance pointer should not be null!");

		glfwInit();

		// If the width or height is 0, we enter the full screen mode.
		if (width == 0 || height == 0)
			m_FullScreenMode = true;
	}

	VkSurfaceCapabilitiesKHR Surface::getCapabilities(const Engine* pEngine) const
	{
		VkSurfaceCapabilitiesKHR vCapabilities = {};
		FIREFLY_VALIDATE(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pEngine->getPhysicalDevice(), m_vSurface, &vCapabilities), "Failed to get the surface capabilities!");

		return vCapabilities;
	}
}