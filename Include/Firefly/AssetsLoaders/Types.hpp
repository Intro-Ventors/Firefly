#pragma once

#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/gtx/hash.hpp>

#include "Firefly/Buffer.hpp"

namespace Firefly
{
	/**
	 * Obj file vertex structure.
	 * This structure represents as a single vertex of an .obj file.
	 */
	struct ObjVertex
	{
		glm::vec3 m_Position = glm::vec3(0.0f);
		glm::vec4 m_Color = glm::vec4(0.0f);
		glm::vec2 m_TextureCoordinate = glm::vec2(0.0f);

		/**
		 * Compare equal to operator.
		 *
		 * @param other The other vector.
		 * @return Boolean value stating if its equal or not.
		 */
		bool operator==(const ObjVertex& other) const
		{
			return m_Position == other.m_Position && m_Color == other.m_Color && m_TextureCoordinate == other.m_TextureCoordinate;
		}
	};

	/**
	 * Obj model structure.
	 * This structure contains the model details of a .obj file.
	 */
	struct ObjModel
	{
		std::shared_ptr<Buffer> m_VertexBuffer = nullptr;
		std::shared_ptr<Buffer> m_IndexBuffer = nullptr;

		uint64_t m_VertexCount = 0;
		uint64_t m_IndexCount = 0;
	};
}

namespace std
{
	template<>
	struct hash<Firefly::ObjVertex>
	{
		size_t operator()(Firefly::ObjVertex const& vertex) const
		{
			return ((hash<glm::vec3>()(vertex.m_Position) ^ (hash<glm::vec4>()(vertex.m_Color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.m_TextureCoordinate) << 1);
		}
	};
}