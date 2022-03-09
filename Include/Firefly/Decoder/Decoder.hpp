#pragma once

#include "Firefly/Engine.hpp"

namespace Firefly
{
	/**
	 * Firefly Decoder class.
	 * This class is the main decoder engine.
	 */
	class Decoder final : public Engine
	{
	public:
		FIREFLY_DEFAULT_COPY(Decoder);
		FIREFLY_DEFAULT_MOVE(Decoder);

		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		explicit Decoder(const std::shared_ptr<Instance>& pInstance)
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR, 
				{ VK_KHR_VIDEO_QUEUE_EXTENSION_NAME , VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME, VK_KHR_VIDEO_DECODE_QUEUE_EXTENSION_NAME, VK_EXT_VIDEO_DECODE_H264_EXTENSION_NAME })
		{
		}

		/**
		 * Create a new decoder.
		 *
		 * @param pInstance The instance pointer.
		 * @rerurn The created decoder pointer.
		 */
		static std::shared_ptr<Decoder> create(const std::shared_ptr<Instance>& pInstance)
		{
			return std::make_shared<Decoder>(pInstance);
		}
	};
}