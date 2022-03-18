#pragma once

#include "Firefly/Graphics/Surface.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

namespace Firefly
{
	/**
	 * Platform specific surface object.
	 * Windows surface object contains the windows specific window APIs and can be used for graphics rendering.
	 */
	class WindowsSurface final : public Surface
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which the surface is bound to.
		 * @param width The surface width.
		 * @param height The surface height.
		 * @param title The surface title.
		 */
		explicit WindowsSurface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::wstring&& title);

		/**
		 * Destructor.
		 */
		~WindowsSurface() override;

		/**
		 * Create a new windows surface.
		 *
		 * @param pInstance The instance pointer to which the surface is bound to.
		 * @param width The surface width.
		 * @param height The surface height.
		 * @param title The surface title.
		 * @return The created window.
		 */
		static std::shared_ptr<WindowsSurface> create(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::wstring&& title);

		/**
		 * Terminate this surface.
		 */
		void terminate() override;

		/**
		 * Get the title from the surface.
		 *
		 * @return The title string.
		 */
		std::wstring_view getTitle() const { return m_Title; }

		/**
		 * Set the surface title.
		 *
		 * @param title The title to be set.
		 */
		void setTitle(const std::wstring& title);

		/**
		 * Set the screen to full screen mode.
		 */
		void setFullScreenMode() override;

		/**
		 * Exit the full screen mode.
		 */
		void exitFullScreenMode() override;

	private:
		/**
		 * Register this class to windows.
		 */
		void registerClass() const;

		/**
		 * Create the Vulkan surface.
		 */
		void createVulkanSurface();

	private:
		std::wstring m_Title;

		HINSTANCE m_HINSTANCE = nullptr;
		HWND m_HWND = nullptr;
		WINDOWPLACEMENT m_Placement;
		LONG m_Style = 0;
		LONG m_StyleEx = 0;
	};
}