#include "Firefly/Graphics/Windows/WindowsSurface.hpp"

#include <system_error>
#include <future>

namespace /* anonymous */
{
	LRESULT CALLBACK MainWndProc(
		HWND hwnd,        // handle to window
		UINT uMsg,        // message identifier
		WPARAM wParam,    // first message parameter
		LPARAM lParam)    // second message parameter
	{
		const auto pSurface = reinterpret_cast<Firefly::WindowsSurface*>(GetClassLongPtrA(hwnd, 0));

		switch (uMsg)
		{
		case WM_CREATE:
			// Initialize the window. 
			return 0;

		case WM_PAINT:
			// Paint the window's client area. 
			return 0;

		case WM_SIZE:
			// Set the size and position of the window. 
			return 0;

		case WM_DESTROY:
			// Clean up window-specific data objects. 
			return 0;

			// 
			// Process other messages. 
			// 

		default:
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

		return 0;
	}
}

namespace Firefly
{
	WindowsSurface::WindowsSurface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::wstring&& title)
		: Surface(pInstance, width, height), m_Title(std::move(title)), m_HINSTANCE(GetModuleHandle(nullptr)), m_Style(WS_CAPTION | WS_MAXIMIZE), m_StyleEx(WS_EX_ACCEPTFILES)
	{
		// Register this class to Windows.
		registerClass();

		// Create the window.
		m_HWND = CreateWindowEx(m_StyleEx, L"WindowsSurface", m_Title.c_str(), m_Style, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, m_HINSTANCE, this);

		// Validate the window creation.
		if (!m_HWND)
			throw BackendError("Failed to create the Windows window! Error message: " + std::system_category().message(GetLastError()));

		// Create the Vulkan surface.
		createVulkanSurface();

		// Show the window.
		ShowWindow(m_HWND, SW_SHOWMAXIMIZED);
	}

	WindowsSurface::~WindowsSurface()
	{
		if (m_HWND)
			terminate();
	}

	std::shared_ptr<WindowsSurface> WindowsSurface::create(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::wstring&& title)
	{
		return std::make_shared<WindowsSurface>(pInstance, width, height, std::move(title));
	}

	void WindowsSurface::terminate()
	{
		vkDestroySurfaceKHR(getInstance()->getInstance(), m_vSurface, nullptr);

		DestroyWindow(m_HWND);
		m_HWND = nullptr;
	}

	void WindowsSurface::setTitle(const std::wstring& title)
	{
		if (SetWindowText(m_HWND, title.c_str()) == 0)
			throw BackendError("Failed to set the window title!");

		m_Title = title;
	}

	void WindowsSurface::setFullScreenMode()
	{
		GetWindowPlacement(m_HWND, &m_Placement);
		if (m_Style == 0)
			m_Style = GetWindowLong(m_HWND, GWL_STYLE);
		if (m_StyleEx == 0)
			m_StyleEx = GetWindowLong(m_HWND, GWL_EXSTYLE);

		const auto newHWNDStyle = m_Style & ~WS_BORDER & ~WS_DLGFRAME & ~WS_THICKFRAME;
		const auto newHWNDStyleEx = m_StyleEx & ~WS_EX_WINDOWEDGE;

		SetWindowLong(m_HWND, GWL_STYLE, newHWNDStyle | WS_POPUP);
		SetWindowLong(m_HWND, GWL_EXSTYLE, newHWNDStyleEx | WS_EX_TOPMOST);
		ShowWindow(m_HWND, SW_SHOWMAXIMIZED);
		m_FullScreenMode = true;
	}

	void WindowsSurface::exitFullScreenMode()
	{
		SetWindowLong(m_HWND, GWL_STYLE, m_Style);
		SetWindowLong(m_HWND, GWL_EXSTYLE, m_StyleEx);
		ShowWindow(m_HWND, SW_SHOWNORMAL);
		SetWindowPlacement(m_HWND, &m_Placement);
		m_FullScreenMode = false;
	}

	void WindowsSurface::registerClass() const
	{
		WNDCLASS wndClasss = {};
		wndClasss.style = CS_HREDRAW | CS_VREDRAW;
		wndClasss.lpfnWndProc = reinterpret_cast<WNDPROC>(MainWndProc);
		wndClasss.cbClsExtra = 0;
		wndClasss.cbWndExtra = 0;
		wndClasss.hInstance = m_HINSTANCE;
		wndClasss.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		wndClasss.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wndClasss.hbrBackground = CreateSolidBrush(0x00000000);
		wndClasss.lpszMenuName = L"MainMenu";
		wndClasss.lpszClassName = L"WindowsSurface";

		if (RegisterClass(&wndClasss) == 0)
			throw BackendError("Failed to register the Windows surface class!");
	}

	void WindowsSurface::createVulkanSurface()
	{
		VkWin32SurfaceCreateInfoKHR vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		vCreateInfo.pNext = nullptr;
		vCreateInfo.flags = 0;
		vCreateInfo.hinstance = m_HINSTANCE;
		vCreateInfo.hwnd = m_HWND;

		FIREFLY_VALIDATE(vkCreateWin32SurfaceKHR(getInstance()->getInstance(), &vCreateInfo, nullptr, &m_vSurface), "Failed to create the Windows surface!");
	}
}
