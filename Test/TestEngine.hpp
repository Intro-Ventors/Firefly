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

	struct Vertex
	{
		struct { float x, y, z; } m_Position;
		struct { float r, g, b, a; } m_Color;
	};

public:
	TestEngine();

	std::shared_ptr<Firefly::Image> draw() const;

private:
	std::vector<Vertex> generateVertices() const;
	std::vector<uint32_t> generateIndices() const;
};