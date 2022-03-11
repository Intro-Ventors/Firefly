#pragma once

#include "Firefly/Instance.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Graphics/RenderTarget.hpp"
#include "Firefly/Graphics/GraphicsPipeline.hpp"
#include "Firefly/Maths/Camera.hpp"

#include "Renderdoc.hpp"

class TestEngine final
{
	Firefly::Camera m_Camera = Firefly::Camera(glm::vec3(0.0f), (1280.0f / 2) / 720.0f);

	std::shared_ptr<Firefly::Instance> m_Instance = nullptr;
	std::shared_ptr<Firefly::GraphicsEngine> m_GraphicsEngine = nullptr;
	std::shared_ptr<Firefly::RenderTarget> m_RenderTarget = nullptr;

	std::shared_ptr<Firefly::Shader> m_VertexShader = nullptr;
	std::shared_ptr<Firefly::Shader> m_FragmentShader = nullptr;
	std::shared_ptr<Firefly::GraphicsPipeline> m_Pipeline = nullptr;

	std::shared_ptr<Firefly::Buffer> m_VertexBuffer = nullptr;
	std::shared_ptr<Firefly::Buffer> m_IndexBuffer = nullptr;

	std::shared_ptr<Firefly::Buffer> m_CameraUniform = nullptr;
	std::shared_ptr<Firefly::Buffer> m_UniformBuffer = nullptr;
	std::shared_ptr<Firefly::Image> m_Texture = nullptr;
	std::shared_ptr<Firefly::Package> m_VertexResourcePackage = nullptr;
	std::shared_ptr<Firefly::Package> m_FragmentResourcePackage = nullptr;

	Renderdoc m_RenderdocIntegration;

	struct Vertex
	{
		struct { float x, y, z; } m_Position;
		struct { float r, g, b, a; } m_Color;
		struct { float u, v; } m_TextureCoordinates;
	};

	uint32_t m_VertexCount = 0;
	uint32_t m_IndexCount = 0;

	bool m_bShouldCapture = false;

public:
	TestEngine();

	std::shared_ptr<Firefly::Image> draw();

	Firefly::Camera& getCamera() { return m_Camera; }
	void captureFrame() { m_bShouldCapture = true; }

private:
	std::vector<Vertex> generateVertices() const;
	std::vector<uint32_t> generateIndices() const;

	std::vector<Vertex> generateQuadVertices() const;
	std::vector<uint32_t> generateQuadIndices() const;
};