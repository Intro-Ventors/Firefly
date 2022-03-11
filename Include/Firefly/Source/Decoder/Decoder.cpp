#include "Firefly/Decoder/Decoder.hpp"

namespace Firefly
{
	Decoder::Decoder(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR,
			{ VK_KHR_VIDEO_QUEUE_EXTENSION_NAME , VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME, VK_EXT_VIDEO_DECODE_H264_EXTENSION_NAME })
	{
	}
	
	std::shared_ptr<Decoder> Decoder::create(const std::shared_ptr<Instance>& pInstance)
	{
		return std::make_shared<Decoder>(pInstance);
	}
}