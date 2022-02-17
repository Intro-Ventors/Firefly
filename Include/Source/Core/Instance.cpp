#include "RCHAC/Core/Instance.hpp"

#include <string>
#include <iostream>
#include <fstream>

namespace RCHAC
{
	namespace /* anonymous */
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
		VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
		{
			std::string myMessagePreStatement = ": ";
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

		/**
		 * Create debug messenger structure.
		 * This utility function will generate the VkDebugUtilsMessengerCreateInfoEXT structure.
		 *
		 * @retrurn The created structure.
		 */
		VkDebugUtilsMessengerCreateInfoEXT CreateDebugMessengerCreateInfo()
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
			createInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
			createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
				| VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
			createInfo.pfnUserCallback = DebugCallback;

			createInfo.pNext = VK_NULL_HANDLE;
			createInfo.pUserData = VK_NULL_HANDLE;
			createInfo.flags = 0;

			return createInfo;
		}
	}

	Instance::Instance(bool enableValidation)
		: m_bEnableValidation(enableValidation)
	{
		// Setup the application info structure.
		VkApplicationInfo vApplicationInfo = {};
		vApplicationInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_APPLICATION_INFO;
		vApplicationInfo.pApplicationName = "RCHAC";
		vApplicationInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.pEngineName = "RCHAC";
		vApplicationInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
		vApplicationInfo.apiVersion = VK_API_VERSION_1_2;

		// Setup the instance create info structure.
		VkInstanceCreateInfo vCreateInfo = {};
		vCreateInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		vCreateInfo.pApplicationInfo = &vApplicationInfo;

		// Get the required extensions.
		const char* pExtensions[] = {
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME
		};

		vCreateInfo.enabledExtensionCount = 1;
		vCreateInfo.ppEnabledExtensionNames = pExtensions;

		// Setup debug info if required.
		VkDebugUtilsMessengerCreateInfoEXT vDebugCreateInfo = {};
		if (m_bEnableValidation)
		{
			// Add the required validation layer.
			m_ValidationLayers.emplace_back("VK_LAYER_KHRONOS_validation");

			// Create the debug messenger.
			vDebugCreateInfo = CreateDebugMessengerCreateInfo();

			vCreateInfo.pNext = &vDebugCreateInfo;
			vCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_ValidationLayers.size());
			vCreateInfo.ppEnabledLayerNames = m_ValidationLayers.data();
		}
		else
		{
			vCreateInfo.enabledLayerCount = 0;
			vCreateInfo.pNext = nullptr;
		}

		// Create the instance.
		Utility::ValidateResult(vkCreateInstance(&vCreateInfo, nullptr, &m_vInstance));

		// Create the debug utils messenger if validation is enabled.
		if (m_bEnableValidation)
		{
			VkDebugUtilsMessengerCreateInfoEXT createInfo = CreateDebugMessengerCreateInfo();

			const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(m_vInstance, "vkCreateDebugUtilsMessengerEXT"));
			Utility::ValidateResult(func(m_vInstance, &createInfo, nullptr, &m_vDebugUtilsMessenger));
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
}