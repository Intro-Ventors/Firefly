#include "Firefly/Utility.hpp"

#include <iostream>

namespace /* anonymous */
{
	/**
	 * Convert from VkResult to a string explaining what the issue is.
	 *
	 * @param result The result.
	 * @return The string explaining it.
	 */
	std::string_view VkResultToString(VkResult result)
	{
		switch (result)
		{
		case VkResult::VK_SUCCESS:													return "{VkResult::VK_SUCCESS} ";
		case VkResult::VK_NOT_READY:												return "{VkResult::VK_NOT_READY} ";
		case VkResult::VK_TIMEOUT:													return "{VkResult::VK_TIMEOUT} ";
		case VkResult::VK_EVENT_SET:												return "{VkResult::VK_EVENT_SET} ";
		case VkResult::VK_EVENT_RESET:												return "{VkResult::VK_EVENT_RESET} ";
		case VkResult::VK_INCOMPLETE:												return "{VkResult::VK_INCOMPLETE} ";
		case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:									return "{VkResult::VK_ERROR_OUT_OF_HOST_MEMORY} ";
		case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY:								return "{VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY} ";
		case VkResult::VK_ERROR_INITIALIZATION_FAILED:								return "{VkResult::VK_ERROR_INITIALIZATION_FAILED} ";
		case VkResult::VK_ERROR_DEVICE_LOST:										return "{VkResult::VK_ERROR_DEVICE_LOST} ";
		case VkResult::VK_ERROR_MEMORY_MAP_FAILED:									return "{VkResult::VK_ERROR_MEMORY_MAP_FAILED} ";
		case VkResult::VK_ERROR_LAYER_NOT_PRESENT:									return "{VkResult::VK_ERROR_LAYER_NOT_PRESENT} ";
		case VkResult::VK_ERROR_EXTENSION_NOT_PRESENT:								return "{VkResult::VK_ERROR_EXTENSION_NOT_PRESENT} ";
		case VkResult::VK_ERROR_FEATURE_NOT_PRESENT:								return "{VkResult::VK_ERROR_FEATURE_NOT_PRESENT} ";
		case VkResult::VK_ERROR_INCOMPATIBLE_DRIVER:								return "{VkResult::VK_ERROR_INCOMPATIBLE_DRIVER} ";
		case VkResult::VK_ERROR_TOO_MANY_OBJECTS:									return "{VkResult::VK_ERROR_TOO_MANY_OBJECTS} ";
		case VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED:								return "{VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED} ";
		case VkResult::VK_ERROR_FRAGMENTED_POOL:									return "{VkResult::VK_ERROR_FRAGMENTED_POOL} ";
		case VkResult::VK_ERROR_UNKNOWN:											return "{VkResult::VK_ERROR_UNKNOWN} ";
		case VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:									return "{VkResult::VK_ERROR_OUT_OF_POOL_MEMORY} ";
		case VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE:							return "{VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE} ";
		case VkResult::VK_ERROR_FRAGMENTATION:										return "{VkResult::VK_ERROR_FRAGMENTATION} ";
		case VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:						return "{VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS} ";
		case VkResult::VK_PIPELINE_COMPILE_REQUIRED:								return "{VkResult::VK_PIPELINE_COMPILE_REQUIRED} ";
		case VkResult::VK_ERROR_SURFACE_LOST_KHR:									return "{VkResult::VK_ERROR_SURFACE_LOST_KHR} ";
		case VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:							return "{VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR} ";
		case VkResult::VK_SUBOPTIMAL_KHR:											return "{VkResult::VK_SUBOPTIMAL_KHR} ";
		case VkResult::VK_ERROR_OUT_OF_DATE_KHR:									return "{VkResult::VK_ERROR_OUT_OF_DATE_KHR} ";
		case VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:							return "{VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR} ";
		case VkResult::VK_ERROR_VALIDATION_FAILED_EXT:								return "{VkResult::VK_ERROR_VALIDATION_FAILED_EXT} ";
		case VkResult::VK_ERROR_INVALID_SHADER_NV:									return "{VkResult::VK_ERROR_INVALID_SHADER_NV} ";
		case VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:		return "{VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT} ";
		case VkResult::VK_ERROR_NOT_PERMITTED_KHR:									return "{VkResult::VK_ERROR_NOT_PERMITTED_KHR} ";
		case VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:				return "{VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT} ";
		case VkResult::VK_THREAD_IDLE_KHR:											return "{VkResult::VK_THREAD_IDLE_KHR} ";
		case VkResult::VK_THREAD_DONE_KHR:											return "{VkResult::VK_THREAD_DONE_KHR} ";
		case VkResult::VK_OPERATION_DEFERRED_KHR:									return "{VkResult::VK_OPERATION_DEFERRED_KHR} ";
		case VkResult::VK_OPERATION_NOT_DEFERRED_KHR:								return "{VkResult::VK_OPERATION_NOT_DEFERRED_KHR} ";
		case VkResult::VK_RESULT_MAX_ENUM:											return "{VkResult::VK_RESULT_MAX_ENUM} ";
		}

		return "{UNKNOWN} ";
	}
}

namespace /* anonymous */
{
	std::function<void(const Firefly::Utility::LogLevel, const std::string_view&)> LoggerFunction;
}

namespace Firefly
{
	namespace Utility
	{
		void ValidateResult(const VkResult result, const std::string& string)
		{
			if (result != VkResult::VK_SUCCESS)
				throw BackendError(VkResultToString(result).data() + string);
		}

		void ValidateResult(const VkResult result, const std::string& string, const std::string_view& file, const uint64_t line)
		{
			if (result != VkResult::VK_SUCCESS)
				throw BackendError(VkResultToString(result).data() + string + " [" + file.data() + ":" + std::to_string(line) + "]");
		}

		void Logger(const VkResult result, const std::string& string, const std::string_view& file, const uint64_t line)
		{
			if (result != VkResult::VK_SUCCESS)
				std::cerr << VkResultToString(result).data() << string << " [" << file.data() << ":" << std::to_string(line) << "]\n";
		}

		void SetLoggerMethod(const std::function<void(const LogLevel level, const std::string_view&)>& function)
		{
			LoggerFunction = function;
		}

		void Log(const LogLevel level, const std::string_view& message)
		{
			// If the user has given us a logger function, lets call it.
			if (LoggerFunction)
				LoggerFunction(level, message);
		}
	}
}
