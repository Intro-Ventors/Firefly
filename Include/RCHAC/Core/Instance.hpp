#pragma once

#include "Engine.hpp"
#include <vector>

namespace RCHAC
{
	/**
	 * RCHAC Instance.
	 * This object contains the main instance of the codec engine.
	 */
	class Instance final : public std::enable_shared_from_this<Instance>
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds.
		 */
		Instance(bool enableValidation);

		/**
		 * Default destructor.
		 */
		~Instance();

		/**
		 * Create a new instance.
		 *
		 * @param enableValidation Whether or not to enable validation. This can slow down the process so it is best advised not to enable this unless on debug builds.
		 * @return The created instance object pointer.
		 */
		static std::shared_ptr<Instance> create(bool enableValidation);

		/**
		 * Check if validation is enabled.
		 *
		 * @return Boolean value stating if validation is enabled.
		 */
		bool isValidationEnabled() const { return m_bEnableValidation; }

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

		/**
		 * Create a new encoder object.
		 *
		 * @return The created encoder engine.
		 */
		std::shared_ptr<Engine> createEncoder();

		/**
		 * Create a new decoder object.
		 *
		 * @return The created decoder engine.
		 */
		std::shared_ptr<Engine> createDecoder();

	private:
		std::vector<const char*> m_ValidationLayers;
		VkInstance m_vInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_vDebugUtilsMessenger = VK_NULL_HANDLE;

		bool m_bEnableValidation = true;
	};
}