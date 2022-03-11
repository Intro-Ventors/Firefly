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
		explicit StereoCamera(const glm::vec3 position, const float aspectRatio)
			: Camera(position, aspectRatio)
		{
			m_LeftEyeMatrix.m_ProjectionMatrix = glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearPlane, m_FarPlane);
			m_LeftEyeMatrix.m_ProjectionMatrix[1][1] *= -1.0f;

			m_RightEyeMatrix = m_LeftEyeMatrix;

			m_FieldOfView = 90.0f;
		}

		/**
		 * Update the matrices.
		 */
		void update() override
		{
			float wd2 = m_NearPlane * tan(glm::radians(m_FieldOfView / 2.0f));
			float ndfl = m_NearPlane / m_FocalLength;
			float left, right;
			float top = wd2;
			float bottom = -wd2;

			// Recalculate the vectors.
			glm::vec3 front;
			front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			front.y = sin(glm::radians(m_Pitch));
			front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_Front = glm::normalize(front);
			m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
			m_Up = glm::normalize(glm::cross(m_Right, m_Front));

			auto rotM = glm::mat4(1.0f);
			//rotM = glm::rotate(rotM, glm::radians(m_Yaw / 2), glm::vec3(1.0f, 0.0f, 0.0f));
			//rotM = glm::rotate(rotM, glm::radians(m_Pitch / 2), glm::vec3(0.0f, 1.0f, 0.0f));
			//rotM = glm::rotate(rotM, glm::radians(camera.rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

			// Left eye
			left = -m_AspectRatio * wd2 + 0.5f * m_EyeSeperation * ndfl;
			right = m_AspectRatio * wd2 + 0.5f * m_EyeSeperation * ndfl;

			const auto transL = glm::translate(glm::mat4(1.0f), m_Position - m_Right * (m_EyeSeperation / 2.0f));

			m_LeftEyeMatrix.m_ProjectionMatrix = glm::frustum(left, right, bottom, top, m_NearPlane, m_FarPlane);
			m_LeftEyeMatrix.m_ViewMatrix = rotM * transL;

			// Right eye
			left = -m_AspectRatio * wd2 - 0.5f * m_EyeSeperation * ndfl;
			right = m_AspectRatio * wd2 - 0.5f * m_EyeSeperation * ndfl;

			const auto transR = glm::translate(glm::mat4(1.0f), m_Position + m_Right * (m_EyeSeperation / 2.0f));

			m_RightEyeMatrix.m_ProjectionMatrix = glm::frustum(left, right, bottom, top, m_NearPlane, m_FarPlane);
			m_RightEyeMatrix.m_ViewMatrix = rotM * transR;
		}

		/**
		 * Copy the matrix to a uniform buffer.
		 *
		 * @param pLeftEyeBuffer The left eye's uniform buffer pointer.
		 * @param pRightEyeBuffer The right eye's uniform buffer pointer.
		 */
		void copyToBuffer(Buffer* pLeftEyeBuffer, Buffer* pRightEyeBuffer) const
		{
			m_LeftEyeMatrix.copyToBuffer(pLeftEyeBuffer);
			m_RightEyeMatrix.copyToBuffer(pRightEyeBuffer);
		}

	public:
		CameraMatrix m_LeftEyeMatrix;
		CameraMatrix m_RightEyeMatrix;

		float m_EyeSeperation = 0.08f;
		float m_FocalLength = 0.5f;
	};
}