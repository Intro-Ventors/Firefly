#include "Firefly/Encoder/Encoder.hpp"

namespace Firefly
{
	Encoder::Encoder(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
			{ VK_KHR_VIDEO_QUEUE_EXTENSION_NAME , VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME, VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME })
	{
	}
	
	std::shared_ptr<Encoder> Encoder::create(const std::shared_ptr<Instance>& pInstance)
	{
		return std::make_shared<Encoder>(pInstance);
	}
}