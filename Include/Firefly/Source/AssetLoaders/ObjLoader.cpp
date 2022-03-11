#include "Firefly/AssetsLoaders/ObjLoader.hpp"

namespace Firefly
{
	ObjModel LoadObjModel(const std::shared_ptr<Engine>& pEngine, const std::filesystem::path& path)
	{
		// Setup the attributes.
		tinyobj::attrib_t attribute;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warning, error;

		// Load the object file.
		if (!tinyobj::LoadObj(&attribute, &shapes, &materials, &warning, &error, path.string().c_str()))
			throw BackendError(warning + error);

		// Setup containers.
		std::unordered_map<ObjVertex, uint32_t> uniqueVertices;
		std::vector<ObjVertex> vertices;
		std::vector<uint32_t> indices;

		// Iterate through the shapes and get the vertices.
		for (const auto& shape : shapes)
		{
			for (const auto& index : shape.mesh.indices)
			{
				ObjVertex vertex;

				vertex.m_Position = {
					attribute.vertices[3 * index.vertex_index + 0],
					attribute.vertices[3 * index.vertex_index + 1],
					attribute.vertices[3 * index.vertex_index + 2]
				};

				vertex.m_TextureCoordinate = {
					attribute.texcoords[2 * index.texcoord_index + 0],
					1.0f - attribute.texcoords[2 * index.texcoord_index + 1]
				};

				vertex.m_Color = { 1.0f, 1.0f, 1.0f, 1.0f };

				if (uniqueVertices.count(vertex) == 0)
				{
					uniqueVertices[vertex] = static_cast<uint32_t>(vertices.size());
					vertices.push_back(vertex);
				}

				indices.push_back(uniqueVertices[vertex]);
			}
		}

		// Create the model structure.
		ObjModel model;

		// Copy vertex data.
		{
			// Create the staging buffer and copy the data to it.
			const auto size = vertices.size() * sizeof(ObjVertex);
			auto pStagingBuffer = Buffer::create(pEngine, size, BufferType::Staging);
			std::copy(vertices.begin(), vertices.end(), reinterpret_cast<ObjVertex*>(pStagingBuffer->mapMemory()));
			pStagingBuffer->unmapMemory();

			// Create the vertex buffer and copy the content to it.
			model.m_VertexCount = vertices.size();
			model.m_VertexBuffer = Buffer::create(pEngine, size, BufferType::Vertex);
			model.m_VertexBuffer->fromBuffer(pStagingBuffer.get());
			vertices.clear();
		}

		// Copy the index data.
		{
			// Create the staging buffer and copy the data to it.
			const auto size = indices.size() * sizeof(uint32_t);
			auto pStagingBuffer = Buffer::create(pEngine, size, BufferType::Staging);
			std::copy(indices.begin(), indices.end(), reinterpret_cast<uint32_t*>(pStagingBuffer->mapMemory()));
			pStagingBuffer->unmapMemory();

			// Create the index buffer and copy the content to it.
			model.m_IndexCount = indices.size();
			model.m_IndexBuffer = Buffer::create(pEngine, indices.size() * sizeof(uint32_t), BufferType::Index);
			model.m_IndexBuffer->fromBuffer(pStagingBuffer.get());
			indices.clear();
		}

		return model;
	}
}