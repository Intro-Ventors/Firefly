#pragma once

#include "Firefly/Instance.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Graphics/RenderTarget.hpp"
#include "Firefly/Graphics/GraphicsPipeline.hpp"

class TestEngine final
{
	std::shared_ptr<Firefly::Instance> m_Instance = nullptr;
	std::shared_ptr<Firefly::GraphicsEngine> m_GraphicsEngine = nullptr;
	std::shared_ptr<Firefly::RenderTarget> m_RenderTarget = nullptr;

	std::shared_ptr<Firefly::Shader> m_VertexShader = nullptr;
	std::shared_ptr<Firefly::Shader> m_FragmentShader = nullptr;
	std::shared_ptr<Firefly::GraphicsPipeline> m_Pipeline = nullptr;

	std::shared_ptr<Firefly::Buffer> m_VertexBuffer = nullptr;
	std::shared_ptr<Firefly::Buffer> m_IndexBuffer = nullptr;

	std::shared_ptr<Firefly::Image> m_Texture = nullptr;
	std::shared_ptr<Firefly::Package> m_ResourcePackage = nullptr;

	struct Vertex
	{
		struct { float x, y, z; } m_Position;
		struct { float r, g, b, a; } m_Color;
		struct { float u, v; } m_TextureCoordinates;
	};

	uint32_t m_VertexCount = 0;
	uint32_t m_IndexCount = 0;

public:
	TestEngine();

	std::shared_ptr<Firefly::Image> draw() const;

private:
	void loadImage();

	std::vector<Vertex> generateVertices() const;
	std::vector<uint32_t> generateIndices() const;

	std::vector<Vertex> generateQuadVertices() const;
	std::vector<uint32_t> generateQuadIndices() const;
};