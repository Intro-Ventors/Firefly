#pragma once

#include "Utility.hpp"
#include <vk_video/vulkan_video_codec_h265std.h>
#include <vector>

namespace RCHAC
{
	/**
	 * RCHAC Instance.
	 * This object contains the main instance of the codec engine.
	 */
	class Instance
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

	private:
		std::vector<const char*> m_ValidationLayers;
		VkInstance m_vInstance = VK_NULL_HANDLE;
		VkDebugUtilsMessengerEXT m_vDebugUtilsMessenger = VK_NULL_HANDLE;
		bool m_bEnableValidation = true;
	};
}