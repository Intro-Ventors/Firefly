#pragma once

#include "RCHAC/Core/Engine.hpp"

namespace RCHAC
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
		Decoder(const std::shared_ptr<Instance>& pInstance);
	};
}