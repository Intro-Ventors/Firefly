#include "RCHAC/Encoder/Encoder.hpp"
#include "RCHAC/Core/Instance.hpp"

namespace RCHAC
{
	Encoder::Encoder(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR)
	{
	}
}