#pragma once

#include "CameraMatrix.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	/**
	 * Camera class.
	 * This class consists of a single camera, and is not recommended for VR applications.
	 */
	class Camera
	{
	public:
		/**
		 * Constructor.
		 *
		 * @param position The camera position in the 3D world.
		 * @pram aspectRatio The camera's aspect ratio.
		 */
		explicit Camera(const glm::vec3 position, const float aspectRatio);

		/**
		 * Move the camera forward.
		 *
		 * @param delta The time delta.
		 */
		void moveForward(const uint64_t delta);

		/**
		 * Move the camera backward.
		 *
		 * @param delta The time delta.
		 */
		void moveBackward(const uint64_t delta);

		/**
		 * Move the camera to the left.
		 *
		 * @param delta The time delta.
		 */
		void moveLeft(const uint64_t delta);

		/**
		 * Move the camera to the right.
		 *
		 * @param delta The time delta.
		 */
		void moveRight(const uint64_t delta);

		/**
		 * Move the camera up.
		 *
		 * @param delta The time delta.
		 */
		void moveUp(const uint64_t delta);

		/**
		 * Move the camera down.
		 *
		 * @param delta The time delta.
		 */
		void moveDown(const uint64_t delta);

		/**
		 * Rotate the camera up.
		 *
		 * @param delta The time delta.
		 */
		void rotateUp(const uint64_t delta);

		/**
		 * Rotate the camera down.
		 *
		 * @param delta The time delta.
		 */
		void rotateDown(const uint64_t delta);

		/**
		 * Rotate the camera to the left.
		 *
		 * @param delta The time delta.
		 */
		void rotateLeft(const uint64_t delta);

		/**
		 * Rotate the camera to the right.
		 *
		 * @param delta The time delta.
		 */
		void rotateRight(const uint64_t delta);

		/**
		 * Update the matrices.
		 */
		virtual void update() = 0;

	public:
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
		float m_NearPlane = 0.001f;

		float m_Yaw = 90.0f;
		float m_Pitch = 0.0f;

		float m_DeltaReductionFactor = 100000000.0f;
	};
}