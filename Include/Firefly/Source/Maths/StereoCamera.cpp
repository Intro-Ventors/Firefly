#include "Firefly/Maths/StereoCamera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace Firefly
{
	StereoCamera::StereoCamera(const glm::vec3 position, const float aspectRatio)
		: Camera(position, aspectRatio, 90.0f)
	{
	}

	void StereoCamera::update()
	{
		float wd2 = m_NearPlane * tan(glm::radians(m_FieldOfView / 2.0f));
		float ndfl = m_NearPlane / m_FocalLength;
		float left = 0.0f;
		float right = 0.0f;
		float top = wd2;
		float bottom = -wd2;

		// Recalculate the vectors.
		auto front = glm::vec3(0);
		front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		front.y = sin(glm::radians(m_Pitch));
		front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
		m_Front = glm::normalize(front);
		m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
		m_Up = glm::normalize(glm::cross(m_Right, m_Front));

		// Left eye
		left = -m_AspectRatio * wd2 + 0.5f * m_EyeSeparation * ndfl;
		right = m_AspectRatio * wd2 + 0.5f * m_EyeSeparation * ndfl;

		m_LeftEyeMatrix.m_ProjectionMatrix = glm::frustum(left, right, bottom, top, m_NearPlane, m_FarPlane);
		m_LeftEyeMatrix.m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position - m_Right * (m_EyeSeparation / 2.0f));

		// Right eye
		left = -m_AspectRatio * wd2 - 0.5f * m_EyeSeparation * ndfl;
		right = m_AspectRatio * wd2 - 0.5f * m_EyeSeparation * ndfl;

		m_RightEyeMatrix.m_ProjectionMatrix = glm::frustum(left, right, bottom, top, m_NearPlane, m_FarPlane);
		m_RightEyeMatrix.m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position + m_Right * (m_EyeSeparation / 2.0f));
	}

	void StereoCamera::copyToBuffer(Buffer* pLeftEyeBuffer, Buffer* pRightEyeBuffer) const
	{
		m_LeftEyeMatrix.copyToBuffer(pLeftEyeBuffer);
		m_RightEyeMatrix.copyToBuffer(pRightEyeBuffer);
	}
}