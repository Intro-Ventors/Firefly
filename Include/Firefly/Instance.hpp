#pragma once

#include "Queue.hpp"

#include <memory>
#include <vector>

namespace Firefly
{
	/**
	 * RCHAC Instance.
	 * This object contains the main instance of the codec engine.
	 */
	class Instance final : public std::enable_shared_from_this<Instance>
	{
	public:
		FIREFLY_NO_COPY(Instance);
		FIREFLY_DEFAULT_MOVE(Instance);

		/**
		 * Constructor.
		 *
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds.
		 * @param vulkanAPIVersion The Vulkan API version to use.
		 * @param usedForGraphics Whether or not this instance is used for graphics.
		 */
		explicit Instance(bool enableValidation, const uint32_t vulkanAPIVersion, bool usedForGraphics);

		/**
		 * Default destructor.
		 */
		~Instance();

		/**
		 * Create a new instance.
		 *
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds.
		 * @param vulkanAPIVersion The Vulkan API version to use.
		 * @param usedForGraphics Whether or not this instance is used for graphics. Default is true.
		 * @return The created instance object pointer.
		 */
		static std::shared_ptr<Instance> create(bool enableValidation, const uint32_t vulkanAPIVersion = VK_API_VERSION_1_3, bool usedForGraphics = true);

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
		/**
		 * Create debug messenger structure.
		 * This utility function will generate the VkDebugUtilsMessengerCreateInfoEXT structure.
		 *
		 * @retrurn The created structure.
		 */
		VkDebugUtilsMessengerCreateInfoEXT createDebugMessengerCreateInfo() const;

	private:
		std::vector<const char*> m_ValidationLayers;

		VkInstance m_vInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_vDebugUtilsMessenger = VK_NULL_HANDLE;
		const uint32_t m_VulkanVersion = 0;

		bool m_bEnableValidation = true;
	};
}