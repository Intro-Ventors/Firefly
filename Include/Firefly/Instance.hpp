#pragma once

#include "Queue.hpp"

#include <memory>
#include <vector>

namespace Firefly
{
	/**
	 * Firefly Instance.
	 * This object contains the main instance of the graphics and codec engines.
	 */
	class Instance final
	{
	public:
		FIREFLY_NO_COPY(Instance);
		FIREFLY_DEFAULT_MOVE(Instance);

		/**
		 * Constructor.
		 *
		 * @param vulkanAPIVersion The Vulkan API version to use.
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds. Default is true.
		 */
		explicit Instance(const uint32_t vulkanAPIVersion, bool enableValidation = true);

		/**
		 * Default destructor.
		 */
		~Instance();

		/**
		 * Create a new instance.
		 *
		 * @param vulkanAPIVersion The Vulkan API version to use. Default is 0.
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds. Default is true.
		 * @return The created instance object pointer.
		 */
		static std::shared_ptr<Instance> create(const uint32_t vulkanAPIVersion = 0, bool enableValidation = true);

		/**
		 * Check if validation is enabled.
		 *
		 * @return Boolean value stating if validation is enabled.
		 */
		bool isValidationEnabled() const { return m_bEnableValidation; }

		/**
		 * Get the Vulkan version used by this instance.
		 *
		 * @return The version.
		 */
		uint32_t getVulkanVersion() const { return m_VulkanVersion; }

		/**
		 * Get the Vulkan instance.
		 *
		 * @return The Vulkan instance.
		 */
		VkInstance getInstance() const { return m_vInstance; }

		/**
		 * Get the Vulkan debugger.
		 *
		 * @return The Vulkan debug utils messenger.
		 */
		VkDebugUtilsMessengerEXT getDebugger() const { return m_vDebugUtilsMessenger; }

		/**
		 * Get the validation layers.
		 *
		 * @return The validation layers.
		 */
		std::vector<const char*> getValidationLayers() const { return m_ValidationLayers; }

	private:
		std::vector<const char*> m_ValidationLayers;

		VkInstance m_vInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_vDebugUtilsMessenger = VK_NULL_HANDLE;
		uint32_t m_VulkanVersion = 0;

		bool m_bEnableValidation = true;
	};
}