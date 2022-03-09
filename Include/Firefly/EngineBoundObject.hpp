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
		FIREFLY_NO_COPY(EngineBoundObject);

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

		/**
		 * Get the engine to which this object is bound to.
		 *
		 * @retrurn The engine pointer.
		 */
		std::shared_ptr<Engine> getEngine() const { return m_pEngine; }

		/**
		 * Check if the object is terminated.
		 * Make sure to toggle this when the object is terminated.
		 *
		 * @return The boolean stating if the object is terminated or not.
		 */
		bool isTerminated() const { return bIsTerminated; }

	protected:
		/**
		 * Toggle the terminated boolean to true.
		 */
		void toggleTerminated() { bIsTerminated = true; }

	private:
		std::shared_ptr<Engine> m_pEngine = nullptr;
		bool bIsTerminated = false;
	};
}