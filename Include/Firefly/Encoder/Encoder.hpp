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
		FIREFLY_DEFAULT_COPY(Encoder);
		FIREFLY_DEFAULT_MOVE(Encoder);

		/**
		 * Constructor.
		 *
		 * @param pInstance The instance pointer to which this object is bound.
		 * @throws std::runtime_error if the instance pointer is null.
		 */
		explicit Encoder(const std::shared_ptr<Instance>& pInstance);

		/**
		 * Create a new encoder.
		 *
		 * @param pInstance The instance pointer.
		 * @rerurn The created encoder pointer.
		 */
		static std::shared_ptr<Encoder> create(const std::shared_ptr<Instance>& pInstance);
	};
}