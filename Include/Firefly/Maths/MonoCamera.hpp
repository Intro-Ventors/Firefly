#pragma once

#include "Camera.hpp"

namespace Firefly
{
	/**
	 * Mono camera object.
	 * These types of cameras only have one view matrix, and is used for normal rendering purposes. For VR applications, use the StereoCamera.
	 */
	class MonoCamera final : public Camera
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param position The camera position in the 3D world.
		 * @pram aspectRatio The camera's aspect ratio.
		 */
		explicit MonoCamera(const glm::vec3 position, const float aspectRatio);

		/**
		 * Update the matrices.
		 */
		void update() override;

		/**
		 * Copy the matrix to a uniform buffer.
		 *
		 * @param pBuffer The uniform buffer pointer.
		 */
		void copyToBuffer(Buffer* pBuffer) const;

	public:
		CameraMatrix m_Matrix;
	};
}