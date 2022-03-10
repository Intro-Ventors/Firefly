#pragma once

#include "CameraMatrix.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	/**
	 * Camera class.
	 * This class consists of a single camera, and is not recommended for VR applications.
	 */
	struct Camera final
	{
		/**
		 * Constructor.
		 *
		 * @param position The camera position in the 3D world.
		 * @pram aspectRatio The camera's aspect ratio.
		 */
		explicit Camera(const glm::vec3 position, const float aspectRatio)
			: m_Position(position), m_AspectRatio(aspectRatio)
		{
			m_Matrix.m_ProjectionMatrix = glm::perspective(glm::radians(m_FieldOfView), m_AspectRatio, m_NearPlane, m_FarPlane);
			m_Matrix.m_ProjectionMatrix[1][1] *= -1.0f;
		}

		/**
		 * Move the camera forward.
		 *
		 * @param delta The time delta.
		 */
		void moveForward(const uint64_t delta)
		{
			m_Position += m_Front * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
		}

		/**
		 * Move the camera backward.
		 *
		 * @param delta The time delta.
		 */
		void moveBackward(const uint64_t delta)
		{
			m_Position -= m_Front * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
		}

		/**
		 * Move the camera to the left.
		 *
		 * @param delta The time delta.
		 */
		void moveLeft(const uint64_t delta)
		{
			m_Position -= m_Right * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
		}

		/**
		 * Move the camera to the right.
		 *
		 * @param delta The time delta.
		 */
		void moveRight(const uint64_t delta)
		{
			m_Position += m_Right * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
		}

		/**
		 * Rotate the camera up.
		 *
		 * @param delta The time delta.
		 */
		void rotateUp(const uint64_t delta)
		{
			m_Pitch += (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
		}

		/**
		 * Rotate the camera down.
		 *
		 * @param delta The time delta.
		 */
		void rotateDown(const uint64_t delta)
		{
			m_Pitch -= (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
		}

		/**
		 * Rotate the camera to the left.
		 *
		 * @param delta The time delta.
		 */
		void rotateLeft(const uint64_t delta)
		{
			m_Yaw += (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
		}

		/**
		 * Rotate the camera to the right.
		 *
		 * @param delta The time delta.
		 */
		void rotateRight(const uint64_t delta)
		{
			m_Yaw -= (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
		}

		/**
		 * Update the matrices.
		 */
		void update()
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
		CameraMatrix m_Matrix = {};

		glm::vec3 m_Position = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_Up = glm::vec3(0.0f, 1.0f, 0.0f);
		glm::vec3 m_Front = glm::vec3(0.0f, 0.0f, -1.0f);
		glm::vec3 m_Right = glm::vec3(1.0f, 0.0f, 0.0f);
		glm::vec3 m_WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);

		float m_MovementBias = 0.005f;
		float m_RotationBias = 0.001f;	// Radians per second.

		float m_FieldOfView = 60.0f;
		float m_AspectRatio = 0.0f;
		float m_FarPlane = 256.0f;
		float m_NearPlane = 1.0f;

		float m_Yaw = 90.0f;
		float m_Pitch = 0.0f;

		float m_DeltaReductionFactor = 100000000.0f;
	};
}