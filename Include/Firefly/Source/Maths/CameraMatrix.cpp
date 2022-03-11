#include "Firefly/Maths/CameraMatrix.hpp"

namespace Firefly
{
	std::shared_ptr<Buffer> CameraMatrix::createBuffer(const std::shared_ptr<GraphicsEngine>& pEngine)
	{
		return std::make_shared<Buffer>(pEngine, sizeof(CameraMatrix), BufferType::Uniform);
	}
	
	void CameraMatrix::copyToBuffer(Buffer* pBuffer) const
	{
		// Validate the buffer.
		if (pBuffer->size() != sizeof(CameraMatrix))
			throw BackendError("The buffer size is not equal to the camera matrix size!");

		// Map the memory and copy this content to it.
		auto pMemory = reinterpret_cast<CameraMatrix*>(pBuffer->mapMemory());
		*pMemory = *this;
		pBuffer->unmapMemory();
	}
}