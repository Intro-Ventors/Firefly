#include "Firefly/Instance.hpp"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <array>

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
		std::string myMessagePreStatement = ": ";
		if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
			myMessagePreStatement += "GENERAL | ";
		else if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
			myMessagePreStatement += "VALIDATION | ";
		else if (messageType & VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
			myMessagePreStatement += "PERFORMANCE | ";

		Utility::LogLevel level = Utility::LogLevel::Information;
		switch (messageSeverity)
		{
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT:
			level = Utility::LogLevel::Warning;
			break;
		case VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT:
			level = Utility::LogLevel::Error;
			break;
		}

		std::stringstream messageStream;
		messageStream << "Vulkan Validation Layer " << myMessagePreStatement << pCallbackData->pMessage;
		Utility::Logger::log(level, messageStream.str());

		return VK_FALSE;
	}

	Instance::Instance(bool enableValidation)
		: m_bEnableValidation(enableValidation)
	{
		// Initialize volk.
		FIREFLY_VALIDATE(volkInitialize(), "Failed to initialize volk!");
		FIREFLY_LOG_INFO("Volk initialized.");

		// Get the instance API version from the driver.
		m_VulkanVersion = volkGetInstanceVersion();

		// Create the instance.
		createInstance();

		// Create the debug utils messenger if validation is enabled.
		if (m_bEnableValidation)
			createDebugger();
	}

	Instance::Instance(const uint32_t vulkanAPIVersion, bool enableValidation)
		: m_VulkanVersion(vulkanAPIVersion), m_bEnableValidation(enableValidation)
	{
		// Initialize volk.
		FIREFLY_VALIDATE(volkInitialize(), "Failed to initialize volk!");
		FIREFLY_LOG_INFO("Volk initialized.");

		// Create the instance.
		createInstance();

		// Create the debug utils messenger if validation is enabled.
		if (m_bEnableValidation)
			createDebugger();
	}

	Instance::~Instance()
	{
		// Destroy the debug utils messenger if created.
		if (m_bEnableValidation)
		{
			// Get the destroyer from the shared library.
			const auto vkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vInstance, "vkDestroyDebugUtilsMessengerEXT"));
			vkDestroyDebugUtilsMessengerEXT(m_vInstance, m_vDebugUtilsMessenger, nullptr);
		}

		// Destroy the Vulkan instance.
		vkDestroyInstance(m_vInstance, nullptr);
	}

	std::shared_ptr<Instance> Instance::create(bool enableValidation)
	{
		return std::make_shared<Instance>(enableValidation);
	}

	std::shared_ptr<Instance> Instance::create(const uint32_t vulkanAPIVersion, bool enableValidation)
	{
		return std::make_shared<Instance>(vulkanAPIVersion, enableValidation);
	}

	void Instance::createInstance()
	{
		// Setup the application info structure.
		VkApplicationInfo vApplicationInfo = {};
		vApplicationInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vApplicationInfo.pApplicationName = "Firefly";
		vApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.pEngineName = "Firefly";
		vApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.apiVersion = m_VulkanVersion;

		// Setup the instance create info structure.
		VkInstanceCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vCreateInfo.pApplicationInfo = &vApplicationInfo;

		// Get the required extensions.
		constexpr const char* extensions[] = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
			VK_KHR_SURFACE_EXTENSION_NAME,

#if defined(_WIN32)
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,

#elif defined(__ANDROID__)
			VK_KHR_ANDROID_SURFACE_EXTENSION_NAME,

#endif
		};

		// Setup debug info if required.
		VkDebugUtilsMessengerCreateInfoEXT vDebugCreateInfo = {};
		if (m_bEnableValidation)
		{
			// Add the required validation layer.
			m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

			// Create the debug messenger.
			vDebugCreateInfo = createDebugMessengerCreateInfo();

			vCreateInfo.pNext = &vDebugCreateInfo;
			vCreateInfo.enabledExtensionCount = sizeof(extensions) / 8;
			vCreateInfo.ppEnabledExtensionNames = extensions;
			vCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			vCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			vCreateInfo.enabledLayerCount = 0;
			vCreateInfo.pNext = nullptr;
		}

		// Create the instance.
		FIREFLY_VALIDATE(vkCreateInstance(&vCreateInfo, nullptr, &m_vInstance), "Failed to create the instance.");
		FIREFLY_LOG_INFO("Instance created.");

		// Load the instance.
		volkLoadInstance(m_vInstance);
		FIREFLY_LOG_INFO("Volk instance loaded.");
	}

	void Instance::createDebugger()
	{
		const auto vCreateInfo = createDebugMessengerCreateInfo();
		const auto vkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vInstance, "vkCreateDebugUtilsMessengerEXT"));
		FIREFLY_VALIDATE(vkCreateDebugUtilsMessengerEXT(m_vInstance, &vCreateInfo, nullptr, &m_vDebugUtilsMessenger), "Failed to create the debug messenger.");
		FIREFLY_LOG_INFO("Debug messenger created.");
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
			= VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
			| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT
			| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
			| VkDebugUtilsMessageSeverityFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

		createInfo.messageType
			= VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
			| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
			| VkDebugUtilsMessageTypeFlagBitsEXT::VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

		return createInfo;
	}
}