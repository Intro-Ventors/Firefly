#pragma once

#include "Firefly/Engine.hpp"

namespace Firefly
{
	/**
	 * Firefly Encoder class.
	 * This class is the main Encoder engine.
	 */
	class Encoder final : public Engine
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		Encoder(const std::shared_ptr<Instance>& pInstance)
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_ENCODE_BIT_KHR,
				{ VK_KHR_VIDEO_QUEUE_EXTENSION_NAME , VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_VIDEO_ENCODE_QUEUE_EXTENSION_NAME, VK_EXT_VIDEO_ENCODE_H264_EXTENSION_NAME })
		{
		}

		/**
		 * Create a new encoder.
		 *
		 * @param pInstance The instance pointer.
		 * @rerurn The created encoder pointer.
		 */
		static std::shared_ptr<Encoder> create(const std::shared_ptr<Instance>& pInstance)
		{
			return std::make_shared<Encoder>(pInstance);
		}
	};
}