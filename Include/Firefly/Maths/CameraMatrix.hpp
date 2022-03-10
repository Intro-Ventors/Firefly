#pragma once

#include <glm/glm.hpp>
#include "Firefly/Buffer.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"

namespace Firefly
{
	/**
	 * Camera matrix structure.
	 * This structure contains information regarding a single camera.
	 */
	struct CameraMatrix
	{
		/**
		 * Create a new buffer which is capable of storing a single camera matrix.
		 *
		 * @param pEngine The graphics engine used to create the buffer.
		 * @return The buffer object.
		 */
		static std::shared_ptr<Buffer> createBuffer(const std::shared_ptr<GraphicsEngine>& pEngine)
		{
			return std::make_shared<Buffer>(pEngine, sizeof(CameraMatrix), BufferType::Uniform);
		}

		/**
		 * Copy the matrix data to a buffer
		 *
		 * @param pBuffer The buffer to which the data is copied.
		 */
		void copyToBuffer(Buffer* pBuffer) const
		{
			// Validate the buffer.
			if (pBuffer->size() != sizeof(CameraMatrix))
				throw BackendError("The buffer size is not equal to the camera matrix size!");

			// Map the memory and copy this content to it.
			auto pMemory = reinterpret_cast<CameraMatrix*>(pBuffer->mapMemory());
			*pMemory = *this;
			pBuffer->unmapMemory();
		}

	public:
		glm::mat4 m_ViewMatrix = glm::mat4(1.0f);
		glm::mat4 m_ProjectionMatrix = glm::mat4(1.0f);
	};
}