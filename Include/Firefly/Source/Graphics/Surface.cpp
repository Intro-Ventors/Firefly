#include "Firefly/Graphics/Surface.hpp"

#include <filesystem>

namespace /* anonymous */
{
	void KeyCallback(GLFWwindow* pWindow, int32_t key, int32_t scancode, int32_t action, int32_t mods)
	{
		static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow))->registerKeyInput(key, action, mods);
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
		static_cast<Firefly::Surface*>(glfwGetWindowUserPointer(pWindow))->registerMouseInput(button, action, mods);
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

	Firefly::InputState GetInputState(const uint32_t action)
	{
		Firefly::InputState state = Firefly::InputState::Released;
		if (action == GLFW_PRESS)		state = Firefly::InputState::Pressed;
		else if (action == GLFW_REPEAT)	state = Firefly::InputState::OnRepeat;

		return state;
	}

	uint8_t GetSpecial(const uint32_t mod)
	{
		uint8_t special = 0;
		if (mod & GLFW_MOD_SHIFT)		special |= static_cast<uint8_t>(Firefly::SpecialKey::Shift);
		if (mod & GLFW_MOD_CONTROL)		special |= static_cast<uint8_t>(Firefly::SpecialKey::Control);
		if (mod & GLFW_MOD_ALT)			special |= static_cast<uint8_t>(Firefly::SpecialKey::Alt);
		if (mod & GLFW_MOD_SUPER)		special |= static_cast<uint8_t>(Firefly::SpecialKey::Super);
		if (mod & GLFW_MOD_CAPS_LOCK)	special |= static_cast<uint8_t>(Firefly::SpecialKey::CapsLock);
		if (mod & GLFW_MOD_NUM_LOCK)	special |= static_cast<uint8_t>(Firefly::SpecialKey::NumLock);

		return special;
	}

	Firefly::KeyInput GetKeyInput(const uint32_t keyCode, const uint32_t action, const uint32_t mod)
	{
		const auto state = GetInputState(action);
		const auto special = GetSpecial(mod);

		switch (keyCode)
		{
		case GLFW_KEY_SPACE:			return Firefly::KeyInput(Firefly::Key::Space, state, special);
		case GLFW_KEY_APOSTROPHE:		return Firefly::KeyInput(Firefly::Key::Apostrophe, state, special);
		case GLFW_KEY_COMMA:			return Firefly::KeyInput(Firefly::Key::Comma, state, special);
		case GLFW_KEY_MINUS:			return Firefly::KeyInput(Firefly::Key::Minus, state, special);
		case GLFW_KEY_PERIOD:			return Firefly::KeyInput(Firefly::Key::Preiod, state, special);
		case GLFW_KEY_SLASH:			return Firefly::KeyInput(Firefly::Key::Slash, state, special);
		case GLFW_KEY_0:				return Firefly::KeyInput(Firefly::Key::Zero, state, special);
		case GLFW_KEY_1:				return Firefly::KeyInput(Firefly::Key::One, state, special);
		case GLFW_KEY_2:				return Firefly::KeyInput(Firefly::Key::Two, state, special);
		case GLFW_KEY_3:				return Firefly::KeyInput(Firefly::Key::Three, state, special);
		case GLFW_KEY_4:				return Firefly::KeyInput(Firefly::Key::Four, state, special);
		case GLFW_KEY_5:				return Firefly::KeyInput(Firefly::Key::Five, state, special);
		case GLFW_KEY_6:				return Firefly::KeyInput(Firefly::Key::Six, state, special);
		case GLFW_KEY_7:				return Firefly::KeyInput(Firefly::Key::Seven, state, special);
		case GLFW_KEY_8:				return Firefly::KeyInput(Firefly::Key::Eight, state, special);
		case GLFW_KEY_9:				return Firefly::KeyInput(Firefly::Key::Nine, state, special);
		case GLFW_KEY_SEMICOLON:		return Firefly::KeyInput(Firefly::Key::Semicolon, state, special);
		case GLFW_KEY_EQUAL:			return Firefly::KeyInput(Firefly::Key::Equal, state, special);
		case GLFW_KEY_A:				return Firefly::KeyInput(Firefly::Key::A, state, special);
		case GLFW_KEY_B:				return Firefly::KeyInput(Firefly::Key::B, state, special);
		case GLFW_KEY_C:				return Firefly::KeyInput(Firefly::Key::C, state, special);
		case GLFW_KEY_D:				return Firefly::KeyInput(Firefly::Key::D, state, special);
		case GLFW_KEY_E:				return Firefly::KeyInput(Firefly::Key::E, state, special);
		case GLFW_KEY_F:				return Firefly::KeyInput(Firefly::Key::F, state, special);
		case GLFW_KEY_G:				return Firefly::KeyInput(Firefly::Key::G, state, special);
		case GLFW_KEY_H:				return Firefly::KeyInput(Firefly::Key::H, state, special);
		case GLFW_KEY_I:				return Firefly::KeyInput(Firefly::Key::I, state, special);
		case GLFW_KEY_J:				return Firefly::KeyInput(Firefly::Key::J, state, special);
		case GLFW_KEY_K:				return Firefly::KeyInput(Firefly::Key::K, state, special);
		case GLFW_KEY_L:				return Firefly::KeyInput(Firefly::Key::L, state, special);
		case GLFW_KEY_M:				return Firefly::KeyInput(Firefly::Key::M, state, special);
		case GLFW_KEY_N:				return Firefly::KeyInput(Firefly::Key::N, state, special);
		case GLFW_KEY_O:				return Firefly::KeyInput(Firefly::Key::O, state, special);
		case GLFW_KEY_P:				return Firefly::KeyInput(Firefly::Key::P, state, special);
		case GLFW_KEY_Q:				return Firefly::KeyInput(Firefly::Key::Q, state, special);
		case GLFW_KEY_R:				return Firefly::KeyInput(Firefly::Key::R, state, special);
		case GLFW_KEY_S:				return Firefly::KeyInput(Firefly::Key::S, state, special);
		case GLFW_KEY_T:				return Firefly::KeyInput(Firefly::Key::T, state, special);
		case GLFW_KEY_U:				return Firefly::KeyInput(Firefly::Key::U, state, special);
		case GLFW_KEY_V:				return Firefly::KeyInput(Firefly::Key::V, state, special);
		case GLFW_KEY_W:				return Firefly::KeyInput(Firefly::Key::W, state, special);
		case GLFW_KEY_X:				return Firefly::KeyInput(Firefly::Key::X, state, special);
		case GLFW_KEY_Y:				return Firefly::KeyInput(Firefly::Key::Y, state, special);
		case GLFW_KEY_Z:				return Firefly::KeyInput(Firefly::Key::Z, state, special);
		case GLFW_KEY_LEFT_BRACKET:		return Firefly::KeyInput(Firefly::Key::LeftBracket, state, special);
		case GLFW_KEY_BACKSLASH:		return Firefly::KeyInput(Firefly::Key::RightBracket, state, special);
		case GLFW_KEY_RIGHT_BRACKET:	return Firefly::KeyInput(Firefly::Key::Backslash, state, special);
		case GLFW_KEY_GRAVE_ACCENT:		return Firefly::KeyInput(Firefly::Key::GraveAccent, state, special);
		case GLFW_KEY_WORLD_1:			return Firefly::KeyInput(Firefly::Key::WordOne, state, special);
		case GLFW_KEY_WORLD_2:			return Firefly::KeyInput(Firefly::Key::WordTwo, state, special);
		case GLFW_KEY_ESCAPE:			return Firefly::KeyInput(Firefly::Key::Escape, state, special);
		case GLFW_KEY_ENTER:			return Firefly::KeyInput(Firefly::Key::Enter, state, special);
		case GLFW_KEY_TAB:				return Firefly::KeyInput(Firefly::Key::Tab, state, special);
		case GLFW_KEY_BACKSPACE:		return Firefly::KeyInput(Firefly::Key::Backspace, state, special);
		case GLFW_KEY_INSERT:			return Firefly::KeyInput(Firefly::Key::Insert, state, special);
		case GLFW_KEY_DELETE:			return Firefly::KeyInput(Firefly::Key::Delete, state, special);
		case GLFW_KEY_RIGHT:			return Firefly::KeyInput(Firefly::Key::Right, state, special);
		case GLFW_KEY_LEFT:				return Firefly::KeyInput(Firefly::Key::Left, state, special);
		case GLFW_KEY_DOWN:				return Firefly::KeyInput(Firefly::Key::Down, state, special);
		case GLFW_KEY_UP:				return Firefly::KeyInput(Firefly::Key::Up, state, special);
		case GLFW_KEY_PAGE_UP:			return Firefly::KeyInput(Firefly::Key::PageUp, state, special);
		case GLFW_KEY_PAGE_DOWN:		return Firefly::KeyInput(Firefly::Key::PageDowm, state, special);
		case GLFW_KEY_HOME:				return Firefly::KeyInput(Firefly::Key::Home, state, special);
		case GLFW_KEY_END:				return Firefly::KeyInput(Firefly::Key::End, state, special);
		case GLFW_KEY_CAPS_LOCK:		return Firefly::KeyInput(Firefly::Key::CapsLock, state, special);
		case GLFW_KEY_SCROLL_LOCK:		return Firefly::KeyInput(Firefly::Key::ScrollLock, state, special);
		case GLFW_KEY_NUM_LOCK:			return Firefly::KeyInput(Firefly::Key::NumLock, state, special);
		case GLFW_KEY_PRINT_SCREEN:		return Firefly::KeyInput(Firefly::Key::PrintScreen, state, special);
		case GLFW_KEY_PAUSE:			return Firefly::KeyInput(Firefly::Key::Pause, state, special);
		case GLFW_KEY_F1:				return Firefly::KeyInput(Firefly::Key::F1, state, special);
		case GLFW_KEY_F2:				return Firefly::KeyInput(Firefly::Key::F2, state, special);
		case GLFW_KEY_F3:				return Firefly::KeyInput(Firefly::Key::F3, state, special);
		case GLFW_KEY_F4:				return Firefly::KeyInput(Firefly::Key::F4, state, special);
		case GLFW_KEY_F5:				return Firefly::KeyInput(Firefly::Key::F5, state, special);
		case GLFW_KEY_F6:				return Firefly::KeyInput(Firefly::Key::F6, state, special);
		case GLFW_KEY_F7:				return Firefly::KeyInput(Firefly::Key::F7, state, special);
		case GLFW_KEY_F8:				return Firefly::KeyInput(Firefly::Key::F8, state, special);
		case GLFW_KEY_F9:				return Firefly::KeyInput(Firefly::Key::F9, state, special);
		case GLFW_KEY_F10:				return Firefly::KeyInput(Firefly::Key::F10, state, special);
		case GLFW_KEY_F11:				return Firefly::KeyInput(Firefly::Key::F11, state, special);
		case GLFW_KEY_F12:				return Firefly::KeyInput(Firefly::Key::F12, state, special);
		case GLFW_KEY_F13:				return Firefly::KeyInput(Firefly::Key::F13, state, special);
		case GLFW_KEY_F14:				return Firefly::KeyInput(Firefly::Key::F14, state, special);
		case GLFW_KEY_F15:				return Firefly::KeyInput(Firefly::Key::F15, state, special);
		case GLFW_KEY_F16:				return Firefly::KeyInput(Firefly::Key::F16, state, special);
		case GLFW_KEY_F17:				return Firefly::KeyInput(Firefly::Key::F17, state, special);
		case GLFW_KEY_F18:				return Firefly::KeyInput(Firefly::Key::F18, state, special);
		case GLFW_KEY_F19:				return Firefly::KeyInput(Firefly::Key::F19, state, special);
		case GLFW_KEY_F20:				return Firefly::KeyInput(Firefly::Key::F20, state, special);
		case GLFW_KEY_F21:				return Firefly::KeyInput(Firefly::Key::F21, state, special);
		case GLFW_KEY_F22:				return Firefly::KeyInput(Firefly::Key::F22, state, special);
		case GLFW_KEY_F23:				return Firefly::KeyInput(Firefly::Key::F23, state, special);
		case GLFW_KEY_F24:				return Firefly::KeyInput(Firefly::Key::F24, state, special);
		case GLFW_KEY_F25:				return Firefly::KeyInput(Firefly::Key::F25, state, special);
		case GLFW_KEY_KP_0:				return Firefly::KeyInput(Firefly::Key::KeyPadZero, state, special);
		case GLFW_KEY_KP_1:				return Firefly::KeyInput(Firefly::Key::KeyPadOne, state, special);
		case GLFW_KEY_KP_2:				return Firefly::KeyInput(Firefly::Key::KeyPadTwo, state, special);
		case GLFW_KEY_KP_3:				return Firefly::KeyInput(Firefly::Key::KeyPadThree, state, special);
		case GLFW_KEY_KP_4:				return Firefly::KeyInput(Firefly::Key::KeyPadFour, state, special);
		case GLFW_KEY_KP_5:				return Firefly::KeyInput(Firefly::Key::KeyPadFive, state, special);
		case GLFW_KEY_KP_6:				return Firefly::KeyInput(Firefly::Key::KeyPadSix, state, special);
		case GLFW_KEY_KP_7:				return Firefly::KeyInput(Firefly::Key::KeyPadSeven, state, special);
		case GLFW_KEY_KP_8:				return Firefly::KeyInput(Firefly::Key::KeyPadEight, state, special);
		case GLFW_KEY_KP_9:				return Firefly::KeyInput(Firefly::Key::KeyPadNine, state, special);
		case GLFW_KEY_KP_DECIMAL:		return Firefly::KeyInput(Firefly::Key::KeyPadDecimal, state, special);
		case GLFW_KEY_KP_DIVIDE:		return Firefly::KeyInput(Firefly::Key::KeyPadDivide, state, special);
		case GLFW_KEY_KP_MULTIPLY:		return Firefly::KeyInput(Firefly::Key::KeyPadMultiply, state, special);
		case GLFW_KEY_KP_SUBTRACT:		return Firefly::KeyInput(Firefly::Key::KeyPadSubtract, state, special);
		case GLFW_KEY_KP_ADD:			return Firefly::KeyInput(Firefly::Key::KeyPadAdd, state, special);
		case GLFW_KEY_KP_ENTER:			return Firefly::KeyInput(Firefly::Key::KeyPadEqual, state, special);
		case GLFW_KEY_KP_EQUAL:			return Firefly::KeyInput(Firefly::Key::KeyPadEnter, state, special);
		case GLFW_KEY_LEFT_SHIFT:		return Firefly::KeyInput(Firefly::Key::LeftShift, state, special);
		case GLFW_KEY_LEFT_CONTROL:		return Firefly::KeyInput(Firefly::Key::LeftControl, state, special);
		case GLFW_KEY_LEFT_ALT:			return Firefly::KeyInput(Firefly::Key::LeftAlt, state, special);
		case GLFW_KEY_LEFT_SUPER:		return Firefly::KeyInput(Firefly::Key::LeftSuper, state, special);
		case GLFW_KEY_RIGHT_SHIFT:		return Firefly::KeyInput(Firefly::Key::RightShift, state, special);
		case GLFW_KEY_RIGHT_CONTROL:	return Firefly::KeyInput(Firefly::Key::RightControl, state, special);
		case GLFW_KEY_RIGHT_ALT:		return Firefly::KeyInput(Firefly::Key::RightAlt, state, special);
		case GLFW_KEY_RIGHT_SUPER:		return Firefly::KeyInput(Firefly::Key::RightSuper, state, special);
		case GLFW_KEY_MENU:				return Firefly::KeyInput(Firefly::Key::Menu, state, special);
		default:						return Firefly::KeyInput(Firefly::Key::Unknown, state, special);
		}
	}

	Firefly::MouseInput GetMouseInput(const uint32_t button, const uint32_t action, const uint32_t mod)
	{
		const auto state = GetInputState(action);
		const auto special = GetSpecial(mod);

		switch (button)
		{
		case GLFW_MOUSE_BUTTON_1:		 return Firefly::MouseInput(Firefly::Button::Left, state, special);
		case GLFW_MOUSE_BUTTON_2:		 return Firefly::MouseInput(Firefly::Button::Right, state, special);
		case GLFW_MOUSE_BUTTON_3:		 return Firefly::MouseInput(Firefly::Button::Middle, state, special);
		case GLFW_MOUSE_BUTTON_4:		 return Firefly::MouseInput(Firefly::Button::Four, state, special);
		case GLFW_MOUSE_BUTTON_5:		 return Firefly::MouseInput(Firefly::Button::Five, state, special);
		case GLFW_MOUSE_BUTTON_6:		 return Firefly::MouseInput(Firefly::Button::Six, state, special);
		case GLFW_MOUSE_BUTTON_7:		 return Firefly::MouseInput(Firefly::Button::Seven, state, special);
		case GLFW_MOUSE_BUTTON_8:		 return Firefly::MouseInput(Firefly::Button::Eight, state, special);
		default:
			break;
		}
	}
}

namespace Firefly
{
	Surface::Surface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::string&& title)
		: m_Title(std::move(title)), m_pInstance(pInstance), m_Width(width), m_Height(height)
	{
		// Check if the instance pointer is valid.
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

	void Surface::registerKeyInput(const uint32_t keyCode, const uint32_t action, const uint32_t mod)
	{
		m_KeyInputs.emplace_back(GetKeyInput(keyCode, action, mod));
	}

	void Surface::registerMouseInput(const uint32_t button, const uint32_t action, const uint32_t mod)
	{
		m_MouseInputs.emplace_back(GetMouseInput(button, action, mod));
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