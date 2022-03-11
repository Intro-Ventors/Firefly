#include "Firefly/Instance.hpp"

#include <iostream>
#include <fstream>

namespace Firefly
{
	/**
	 * Vulkan debug callback.
	 * This function will be called when a debug message is to be printed by the Vulkan framework.
	 *
	 * @param messageSeverity The severity of the message.
	 * @param messageType The type of the message.
	 * @param pCallbackData The callback data. This may contain valuable information regarding an error.
	 * @param pUseData The user data that was provided at the time of the error.
	 * @return A boolean value.
	 */
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
		VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData)
	{
		std::string_view myMessagePreStatement = ": ";
		static std::ofstream generalOutputFile("VulkanGeneralOutput.txt");

		if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
		{
			myMessagePreStatement = "(General): ";
			generalOutputFile << "Vulkan Validation Layer " << myMessagePreStatement << pCallbackData->pMessage << std::endl;

			return VK_FALSE;
		}
		else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			myMessagePreStatement = "(Validation): ";
		else if (messageType == VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			myMessagePreStatement = "(Performance): ";

		std::cout << "Vulkan Validation Layer " << myMessagePreStatement << pCallbackData->pMessage << std::endl;
		return VK_FALSE;
	}
	
	Instance::Instance(bool enableValidation, const uint32_t vulkanAPIVersion, bool usedForGraphics)
		: m_VulkanVersion(vulkanAPIVersion), m_bEnableValidation(enableValidation)
	{
		// Initialize volk.
		Utility::ValidateResult(volkInitialize(), "Failed to initialize volk!");

		// Setup the application info structure.
		VkApplicationInfo vApplicationInfo = {};
		vApplicationInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vApplicationInfo.pApplicationName = "GraphicsCore";
		vApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.pEngineName = "GraphicsCore";
		vApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.apiVersion = vulkanAPIVersion;

		// Setup the instance create info structure.
		VkInstanceCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vCreateInfo.pApplicationInfo = &vApplicationInfo;

		// Get the required extensions.
		std::vector<const char*> extensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME };

		if (usedForGraphics)
			extensions.emplace_back(VK_KHR_SURFACE_EXTENSION_NAME);

		// Setup debug info if required.
		VkDebugUtilsMessengerCreateInfoEXT vDebugCreateInfo = {};
		if (m_bEnableValidation)
		{
			// Add the required validation layer.
			m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

			// Create the debug messenger.
			vDebugCreateInfo = createDebugMessengerCreateInfo();

			vCreateInfo.pNext = &vDebugCreateInfo;
			vCreateInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
			vCreateInfo.ppEnabledExtensionNames = extensions.data();
			vCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			vCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			vCreateInfo.enabledLayerCount = 0;
			vCreateInfo.pNext = nullptr;
		}

		// Create the instance.
		Utility::ValidateResult(vkCreateInstance(&vCreateInfo, nullptr, &m_vInstance), "Failed to create the instance.");

		// Load the instance.
		volkLoadInstance(m_vInstance);

		// Create the debug utils messenger if validation is enabled.
		if (m_bEnableValidation)
		{
			VkDebugUtilsMessengerCreateInfoEXT vCreateInfo = createDebugMessengerCreateInfo();

			const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vInstance, "vkCreateDebugUtilsMessengerEXT"));
			Utility::ValidateResult(func(m_vInstance, &vCreateInfo, nullptr, &m_vDebugUtilsMessenger), "Failed to create the debug messenger.");
		}
	}

	Instance::~Instance()
	{
		// Destroy the debug utils messenger if created.
		if (m_bEnableValidation)
		{
			// Get the destroyer from the shared library.
			const auto destroyer = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vInstance, "vkDestroyDebugUtilsMessengerEXT"));

			if (destroyer)
				destroyer(m_vInstance, m_vDebugUtilsMessenger, nullptr);
		}

		// Destroy the Vulkan instance.
		vkDestroyInstance(m_vInstance, nullptr);
	}
	
	std::shared_ptr<Instance> Instance::create(bool enableValidation, const uint32_t vulkanAPIVersion, bool usedForGraphics)
	{
		return std::make_shared<Instance>(enableValidation, vulkanAPIVersion, usedForGraphics);
	}
	
	VkDebugUtilsMessengerCreateInfoEXT Instance::createDebugMessengerCreateInfo() const
	{
		VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
		createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		createInfo.pNext = VK_NULL_HANDLE;
		createInfo.pUserData = VK_NULL_HANDLE;
		createInfo.flags = 0;
		createInfo.pfnUserCallback = DebugCallback;

		createInfo.messageSeverity
			= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType
			= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		return createInfo;
	}
}