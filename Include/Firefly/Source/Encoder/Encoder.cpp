#include "Firefly/Encoder/Encoder.hpp"

namespace Firefly
{
	Encoder::Encoder(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance)
	{
	}
	
	std::shared_ptr<Encoder> Encoder::create(const std::shared_ptr<Instance>& pInstance)
	{
		const auto pointer = std::make_shared<Encoder>(pInstance);
		FIREFLY_VALIDATE_OBJECT(pointer);

		pointer->initialize(VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR, { VK_KHR_VIDEO_QUEUE_EXTENSION_NAME , VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME, VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME });

		return pointer;
	}
}