#pragma once

#include "Camera.hpp"

namespace Firefly
{
	/**
	 * Stereo camera object.
	 * These types of cameras have two view and projection matrices used each eye. This is intended to be used for VR applications.
	 */
	class StereoCamera final : public Camera
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param position The camera position in the 3D world.
		 * @pram aspectRatio The camera's aspect ratio.
		 */
		explicit StereoCamera(const glm::vec3 position, const float aspectRatio);

		/**
		 * Update the matrices.
		 */
		void update() override;

		/**
		 * Copy the matrix to a uniform buffer.
		 *
		 * @param pLeftEyeBuffer The left eye's uniform buffer pointer.
		 * @param pRightEyeBuffer The right eye's uniform buffer pointer.
		 */
		void copyToBuffer(Buffer* pLeftEyeBuffer, Buffer* pRightEyeBuffer) const;

	public:
		CameraMatrix m_LeftEyeMatrix = {};
		CameraMatrix m_RightEyeMatrix = {};

		float m_EyeSeparation = 0.08f;
		float m_FocalLength = 0.5f;
	};
}