#include "RCHAC/Decoder/Decoder.hpp"
#include "RCHAC/Core/Instance.hpp"

namespace RCHAC
{
	Decoder::Decoder(const std::shared_ptr<Instance>& pInstance)
		: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
	{
	}
}