#pragma once

#include "Engine.hpp"

namespace Firefly
{
	/**
	 * Engine bound object.
	 * This object is the base class for all the engine bound objects.
	 */
	class EngineBoundObject
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param pEngine The engine pointer. Make sure that this pointer is not null.
		 */
		explicit EngineBoundObject(const std::shared_ptr<Engine>& pEngine) : m_pEngine(pEngine)
		{
			// Validate the engine pointer.
			if (!pEngine)
				throw BackendError("The engine pointer cannot be null!");
		}

		/**
		 * Default destructor.
		 */
		virtual ~EngineBoundObject() = default;

		/**
		 * Terminate the object.
		 */
		virtual void terminate() = 0;

	protected:
		std::shared_ptr<Engine> m_pEngine = nullptr;
	};
}