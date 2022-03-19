#include "Firefly/Graphics/Surface.hpp"

#include <filesystem>

namespace /* anonymous */
{
	void KeyCallback(GLFWwindow* pWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->ActivateKey(scancode, action, static_cast<SpecialCharacter>(mods));
	}

	void TextCallback(GLFWwindow* pWindow, uint32_t codepoint)
	{
	}

	void CursorPositionCallback(GLFWwindow* pWindow, double xOffset, double yOffset)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->SetMousePosition(static_cast<float>(xOffset), static_cast<float>(yOffset));
	}

	void MouseButtonCallback(GLFWwindow* pWindow, int32_t button, int32_t action, int32_t mods)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->ActivateMouseButton(button, action, static_cast<SpecialCharacter>(mods));
	}

	void MouseScrollCallback(GLFWwindow* pWindow, double xOffset, double yOffset)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->SetMouseScroll(static_cast<float>(xOffset), static_cast<float>(yOffset));
	}

	void MouseCursorEnterCallback(GLFWwindow* pWindow, int32_t entered)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->SetCursorWithinDisplay(entered == GLFW_TRUE);
	}

	void ApplicationDropPathCallback(GLFWwindow* pWindow, int32_t count, const char** strings)
	{
		std::vector<std::filesystem::path> paths(count);
		for (uint32_t i = 0; i < static_cast<uint32_t>(count); i++)
			paths[i] = strings[i];

		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->SetDragAndDropPaths(std::move(paths));
	}

	void ApplicationResizeCallback(GLFWwindow* pWindow, int32_t width, int32_t height)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->SetNewExtent(FBox2D(width, height));
		//pSurface->Resized();
	}

	void WindowCloseCallback(GLFWwindow* pWindow)
	{
		auto pSurface = static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow));
		//pSurface->ToggleClose();
	}

	void GLFWErrorCallback(int code, const char* description) 
	{
		Firefly::Utility::Logger::log(Firefly::Utility::LogLevel::Error, description);
	}
}

namespace Firefly
{
	Surface::Surface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::string&& title)
		: m_Title(std::move(title)), m_pInstance(pInstance), m_Width(width), m_Height(height)
	{
		if (!pInstance)
			throw BackendError("The instance pointer should not be null!");

		// If the width or height is 0, we enter the full screen mode.
		if (width == 0 || height == 0)
			m_FullScreenMode = true;

		// Setup glfw.
		setupGLFW();

		// Create the surface.
		createSurface();
	}

	Surface::~Surface()
	{
		if (!m_IsTerminated)
			terminate();

		glfwTerminate();
	}

	std::shared_ptr<Surface> Surface::create(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::string&& title)
	{
		return std::make_shared<Surface>(pInstance, width, height, std::move(title));
	}

	VkSurfaceCapabilitiesKHR Surface::getCapabilities(const Engine* pEngine) const
	{
		VkSurfaceCapabilitiesKHR vCapabilities = {};
		FIREFLY_VALIDATE(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(pEngine->getPhysicalDevice(), m_vSurface, &vCapabilities), "Failed to get the surface capabilities!");

		return vCapabilities;
	}

	void Surface::update() const
	{
		glfwPollEvents();
	}

	void Surface::terminate()
	{
		vkDestroySurfaceKHR(getInstance()->getInstance(), m_vSurface, nullptr);
		glfwDestroyWindow(m_pWindow);
		m_IsTerminated = true;
	}

	void Surface::resize(const uint32_t width, const uint32_t height)
	{
		glfwSetWindowSize(m_pWindow, static_cast<int>(width), static_cast<int>(height));
		m_Width = width;
		m_Height = height;
	}

	void Surface::setFullScreenMode()
	{
		glfwMaximizeWindow(m_pWindow);
		m_FullScreenMode = true;
	}

	void Surface::exitFullScreenMode()
	{
		glfwRestoreWindow(m_pWindow);
		m_FullScreenMode = true;
	}

	void Surface::setTitle(const std::string& title)
	{
		glfwSetWindowTitle(m_pWindow, title.c_str());
		m_Title = title;
	}

	void Surface::setupGLFW()
	{
		// Initialize glfw.
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwSetErrorCallback(GLFWErrorCallback);

		const auto pMonitor = glfwGetPrimaryMonitor();
		const auto pMode = glfwGetVideoMode(pMonitor);

		// If we need the screen in full screen, lets do so.
		if (m_FullScreenMode)
		{
			glfwWindowHint(GLFW_RED_BITS, pMode->redBits);
			glfwWindowHint(GLFW_GREEN_BITS, pMode->greenBits);
			glfwWindowHint(GLFW_BLUE_BITS, pMode->blueBits);
			glfwWindowHint(GLFW_REFRESH_RATE, pMode->refreshRate);

			// Create the pWindow.
			m_pWindow = glfwCreateWindow(pMode->width, pMode->height, m_Title.c_str(), pMonitor, nullptr);
			m_Width = pMode->width;
			m_Height = pMode->height;
		}
		else
		{
			// If the width or height is more than the supported, lets default to it.
			if (m_Width > static_cast<uint32_t>(pMode->width) || m_Height > static_cast<uint32_t>(pMode->height))
			{
				// Create the pWindow.
				m_pWindow = glfwCreateWindow(1280, 720, m_Title.c_str(), nullptr, nullptr);
				glfwMaximizeWindow(m_pWindow);

				// Get the new size and assign it to the variables.
				int32_t width = 0, height = 0;
				glfwGetWindowSize(m_pWindow, &width, &height);

				m_Width = width;
				m_Height = height;
			}
			else
			{
				// Else no worries, we can normally create the pWindow.
				m_pWindow = glfwCreateWindow(m_Width, m_Height, m_Title.c_str(), nullptr, nullptr);
			}
		}

		// Validate the pWindow creation.
		if (!m_pWindow)
			throw BackendError("Failed to create display!");

		// Setup the callbacks.
		glfwSetWindowUserPointer(m_pWindow, this);
		glfwSetKeyCallback(m_pWindow, KeyCallback);
		glfwSetCursorPosCallback(m_pWindow, CursorPositionCallback);
		glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
		glfwSetScrollCallback(m_pWindow, MouseScrollCallback);
		glfwSetCursorEnterCallback(m_pWindow, MouseCursorEnterCallback);
		glfwSetDropCallback(m_pWindow, ApplicationDropPathCallback);
		glfwSetWindowCloseCallback(m_pWindow, WindowCloseCallback);
		glfwSetWindowSizeCallback(m_pWindow, ApplicationResizeCallback);
	}
	
	void Surface::createSurface()
	{
		FIREFLY_VALIDATE(glfwCreateWindowSurface(getInstance()->getInstance(), m_pWindow, nullptr, &m_vSurface), "Failed to create the Vulkan surface!");
	}
}