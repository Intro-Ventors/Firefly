#pragma once

#include "RCHAC/Core/Engine.hpp"

namespace RCHAC
{
	/**
	 * RCHAC Encoder class.
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
		Encoder(const std::shared_ptr<Instance>& pInstance);
	};
}