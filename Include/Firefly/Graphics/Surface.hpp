#pragma once

#include "GraphicsEngine.hpp"

namespace Firefly
{
	/**
	 * Firefly surface class.
	 * This object acts as a single surface to which Vulkan renders to.
	 */
	class Surface
	{
	public:
		/**
		 * Constructor.
		 * If the width and/ or height is 0, we default to full screen mode.
		 *
		 * @param pInstance The instance pointer to which the surface is bound to.
		 * @param width The surface width.
		 * @param height The surface height.
		 */
		explicit Surface(const std::shared_ptr<Instance>& pInstance, const uint32_t width, const uint32_t height);

		/**
		 * Virtual destructor.
		 */
		virtual ~Surface() = default;

		/**
		 * Get the surface capabilities.
		 *
		 * @param pEngine The engine pointer to get the capabilities from.
		 * @return The surface capabilities.
		 */
		VkSurfaceCapabilitiesKHR getCapabilities(const Engine* pEngine) const;

		/**
		 * Terminate the surface.
		 */
		virtual void terminate() = 0;

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
		 * Check if the screen is in full screen mode.
		 *
		 * @return Boolean stating if we are in the full screen mode or not.
		 */
		bool isInFullScreenMode() const { return m_FullScreenMode; }

		/**
		 * Set the screen to full screen mode.
		 */
		virtual void setFullScreenMode() = 0;

		/**
		 * Exit the full screen mode.
		 */
		virtual void exitFullScreenMode() = 0;

		/**
		 * Get the Vulkan surface.
		 *
		 * @return The Vulkan surface.
		 */
		VkSurfaceKHR getSurface() const { return m_vSurface; }

	protected:
		std::shared_ptr<Instance> m_pInstance = nullptr;

		VkSurfaceKHR m_vSurface = VK_NULL_HANDLE;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;

		bool m_FullScreenMode = false;
	};
}