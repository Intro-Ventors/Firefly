#include "Firefly/Maths/Camera.hpp"

namespace Firefly
{
	Camera::Camera(const glm::vec3 position, const float aspectRatio, const float fieldOfView)
		: m_Position(position), m_AspectRatio(aspectRatio), m_FieldOfView(fieldOfView)
	{
	}

	void Camera::moveForward(const uint64_t delta)
	{
		m_Position += m_Front * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::moveBackward(const uint64_t delta)
	{
		m_Position -= m_Front * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::moveLeft(const uint64_t delta)
	{
		m_Position -= m_Right * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::moveRight(const uint64_t delta)
	{
		m_Position += m_Right * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::moveUp(const uint64_t delta)
	{
		m_Position += m_Up * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::moveDown(const uint64_t delta)
	{
		m_Position -= m_Up * (static_cast<float>(delta) / m_DeltaReductionFactor) * m_MovementBias;
	}
	
	void Camera::rotateUp(const uint64_t delta)
	{
		m_Pitch += (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
	}
	
	void Camera::rotateDown(const uint64_t delta)
	{
		m_Pitch -= (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
	}
	
	void Camera::rotateLeft(const uint64_t delta)
	{
		m_Yaw += (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
	}
	
	void Camera::rotateRight(const uint64_t delta)
	{
		m_Yaw -= (static_cast<float>(delta) / m_DeltaReductionFactor) * m_RotationBias;
	}
}