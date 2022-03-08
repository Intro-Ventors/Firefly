#pragma once

#include "GraphicsCore/Engine.hpp"

namespace GraphicsCore
{
	/**
	 * RCHAC Decoder class.
	 * This class is the main decoder engine.
	 */
	class Decoder final : public Engine
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		Decoder(const std::shared_ptr<Instance>& pInstance)
			: Engine(pInstance, VkQueueFlagBits::VK_QUEUE_VIDEO_DECODE_BIT_KHR)
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