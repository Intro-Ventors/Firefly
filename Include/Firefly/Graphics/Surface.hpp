#pragma once

#include "GraphicsEngine.hpp"

#define GLFW_DLL
#include <GLFW/glfw3.h>

namespace Firefly
{
	/**
	 * Firefly surface class.
	 * This object acts as a single surface to which Vulkan renders to.
	 */
	class Surface final
	{
	public:
		/**
		 * Constructor.
		 * If the width and/ or height is 0, we default to full screen mode.
		 *
		 * @param pInstance The instance pointer to which the surface is bound to.
		 * @param width The surface width.
		 * @param height The surface height.
		 * @param title The title of the surface.
		 */
		explicit Surface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::string&& title);

		/**
		 * Virtual destructor.
		 */
		~Surface();

		/**
		 * Create a new surface object.
		 *
		 * @param pInstance The instance pointer to which the surface is bound to.
		 * @param width The surface width.
		 * @param height The surface height.
		 * @param title The title of the surface.
		 * @return The surface object.
		 */
		static std::shared_ptr<Surface> create(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height, std::string&& title);

		/**
		 * Get the surface capabilities.
		 *
		 * @param pEngine The engine pointer to get the capabilities from.
		 * @return The surface capabilities.
		 */
		VkSurfaceCapabilitiesKHR getCapabilities(const Engine* pEngine) const;

		/**
		 * Update the surface.
		 * This will poll all the events.
		 */
		void update() const;

		/**
		 * Terminate the surface.
		 */
		void terminate();

		/**
		 * Get the instance pointer.
		 *
		 * @return The instance pointer.
		 */
		std::shared_ptr<Instance> getInstance() const { return m_pInstance; }

		/**
		 * Get the width of the surface.
		 *
		 * @return The width.
		 */
		uint32_t getWidth() const { return m_Width; }

		/**
		 * Get the height of the surface.
		 *
		 * @return The height.
		 */
		uint32_t getHeight() const { return m_Height; }

		/**
		 * Resize the surface.
		 * 
		 * @param width The width of the surface to be set.
		 * @param height The height of the surface to be set.
		 */
		void resize(const uint32_t width, const uint32_t height);

		/**
		 * Check if the screen is in full screen mode.
		 *
		 * @return Boolean stating if we are in the full screen mode or not.
		 */
		bool isInFullScreenMode() const { return m_FullScreenMode; }

		/**
		 * Set the screen to full screen mode.
		 */
		void setFullScreenMode();

		/**
		 * Exit the full screen mode.
		 */
		void exitFullScreenMode();

		/**
		 * Get the Vulkan surface.
		 *
		 * @return The Vulkan surface.
		 */
		VkSurfaceKHR getSurface() const { return m_vSurface; }

		/**
		 * Get the title from the surface.
		 *
		 * @return The title string.
		 */
		std::string_view getTitle() const { return m_Title; }

		/**
		 * Set the surface title.
		 *
		 * @param title The title to be set.
		 */
		void setTitle(const std::string& title);

	private:
		/**
		 * Setup and create the GLFW window.
		 */
		void setupGLFW();

		/**
		 * Create the Vulkan surface.
		 */
		void createSurface();

	private:
		std::string m_Title;
		std::shared_ptr<Instance> m_pInstance = nullptr;

		GLFWwindow* m_pWindow = nullptr;
		VkSurfaceKHR m_vSurface = VK_NULL_HANDLE;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		bool m_FullScreenMode = false;
		bool m_IsTerminated = false;
	};
}