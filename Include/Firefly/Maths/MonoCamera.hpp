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
		explicit MonoCamera(const glm::vec3 position, const float aspectRatio)
			: Camera(position, aspectRatio)
		{
			m_Matrix.m_ProjectionMatrix = glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearPlane, m_FarPlane);
			m_Matrix.m_ProjectionMatrix[1][1] *= -1.0f;
		}

		/**
		 * Update the matrices.
		 */
		void update() override
		{
			// Recalculate the vectors.
			glm::vec3 front;
			front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			front.y = sin(glm::radians(m_Pitch));
			front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
			m_Front = glm::normalize(front);
			m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
			m_Up = glm::normalize(glm::cross(m_Right, m_Front));

			// Calculate the matrices.
			m_Matrix.m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_Front, m_Up);
			m_Matrix.m_ProjectionMatrix = glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearPlane, m_FarPlane);
			m_Matrix.m_ProjectionMatrix[1][1] *= -1.0f;
		}

		/**
		 * Copy the matrix to a uniform buffer.
		 *
		 * @param pBuffer The uniform buffer pointer.
		 */
		void copyToBuffer(Buffer* pBuffer) const { m_Matrix.copyToBuffer(pBuffer); }

	public:
		CameraMatrix m_Matrix;
	};
}