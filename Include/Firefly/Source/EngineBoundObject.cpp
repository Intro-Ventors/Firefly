#include "Firefly/EngineBoundObject.hpp"

namespace Firefly
{
	EngineBoundObject::EngineBoundObject(const std::shared_ptr<Engine>& pEngine)
		: m_pEngine(pEngine)
	{
		// Validate the engine pointer.
		if (!pEngine)
			throw BackendError("The engine pointer cannot be null!");
	}
}