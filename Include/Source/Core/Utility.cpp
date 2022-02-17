#include "RCHAC/Core/Utility.hpp"

#include <stdexcept>

#define ERROR_MESSAGE(code)	"Validation failed with: " #code

namespace RCHAC
{
	namespace Utility
	{
		void ValidateResult(const VkResult result)
		{
			switch (result)
			{
			case VkResult::VK_SUCCESS:												return;
			case VkResult::VK_NOT_READY:											throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_NOT_READY));
			case VkResult::VK_TIMEOUT:												throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_TIMEOUT));
			case VkResult::VK_EVENT_SET:											throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_EVENT_SET));
			case VkResult::VK_EVENT_RESET:											throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_EVENT_RESET));
			case VkResult::VK_INCOMPLETE:											throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_INCOMPLETE));
			case VkResult::VK_ERROR_OUT_OF_HOST_MEMORY:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_OUT_OF_HOST_MEMORY));
			case VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_OUT_OF_DEVICE_MEMORY));
			case VkResult::VK_ERROR_INITIALIZATION_FAILED:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INITIALIZATION_FAILED));
			case VkResult::VK_ERROR_DEVICE_LOST:									throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_DEVICE_LOST));
			case VkResult::VK_ERROR_MEMORY_MAP_FAILED:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_MEMORY_MAP_FAILED));
			case VkResult::VK_ERROR_LAYER_NOT_PRESENT:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_LAYER_NOT_PRESENT));
			case VkResult::VK_ERROR_EXTENSION_NOT_PRESENT:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_EXTENSION_NOT_PRESENT));
			case VkResult::VK_ERROR_FEATURE_NOT_PRESENT:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_FEATURE_NOT_PRESENT));
			case VkResult::VK_ERROR_INCOMPATIBLE_DRIVER:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INCOMPATIBLE_DRIVER));
			case VkResult::VK_ERROR_TOO_MANY_OBJECTS:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_TOO_MANY_OBJECTS));
			case VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_FORMAT_NOT_SUPPORTED));
			case VkResult::VK_ERROR_FRAGMENTED_POOL:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_FRAGMENTED_POOL));
			case VkResult::VK_ERROR_UNKNOWN:										throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_UNKNOWN));
			case VkResult::VK_ERROR_OUT_OF_POOL_MEMORY:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_OUT_OF_POOL_MEMORY));
			case VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE:						throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INVALID_EXTERNAL_HANDLE));
			case VkResult::VK_ERROR_FRAGMENTATION:									throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_FRAGMENTATION));
			case VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS:					throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INVALID_OPAQUE_CAPTURE_ADDRESS));
			case VkResult::VK_ERROR_SURFACE_LOST_KHR:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_SURFACE_LOST_KHR));
			case VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:						throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_NATIVE_WINDOW_IN_USE_KHR));
			case VkResult::VK_SUBOPTIMAL_KHR:										throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_SUBOPTIMAL_KHR));
			case VkResult::VK_ERROR_OUT_OF_DATE_KHR:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_OUT_OF_DATE_KHR));
			case VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:						throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INCOMPATIBLE_DISPLAY_KHR));
			case VkResult::VK_ERROR_VALIDATION_FAILED_EXT:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_VALIDATION_FAILED_EXT));
			case VkResult::VK_ERROR_INVALID_SHADER_NV:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INVALID_SHADER_NV));
			case VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT:	throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_INVALID_DRM_FORMAT_MODIFIER_PLANE_LAYOUT_EXT));
			case VkResult::VK_ERROR_NOT_PERMITTED_EXT:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_NOT_PERMITTED_EXT));
			case VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT:			throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_ERROR_FULL_SCREEN_EXCLUSIVE_MODE_LOST_EXT));
			case VkResult::VK_THREAD_IDLE_KHR:										throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_THREAD_IDLE_KHR));
			case VkResult::VK_THREAD_DONE_KHR:										throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_THREAD_DONE_KHR));
			case VkResult::VK_OPERATION_DEFERRED_KHR:								throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_OPERATION_DEFERRED_KHR));
			case VkResult::VK_OPERATION_NOT_DEFERRED_KHR:							throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_OPERATION_NOT_DEFERRED_KHR));
			case VkResult::VK_PIPELINE_COMPILE_REQUIRED_EXT:						throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_PIPELINE_COMPILE_REQUIRED_EXT));
			case VkResult::VK_RESULT_MAX_ENUM:										throw std::runtime_error(ERROR_MESSAGE(VkResult::VK_RESULT_MAX_ENUM));
			default:																throw std::runtime_error("Unknown error!");
			}
		}
	}
}