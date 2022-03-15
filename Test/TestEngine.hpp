#pragma once

#include "Firefly/Instance.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Graphics/RenderTarget.hpp"
#include "Firefly/Graphics/GraphicsPipeline.hpp"
#include "Firefly/Maths/MonoCamera.hpp"
#include "Firefly/Maths/StereoCamera.hpp"

#include "Renderdoc.hpp"

class TestEngine final
{
	Firefly::StereoCamera m_Camera = Firefly::StereoCamera(glm::vec3(0.0f), (1280.0f / 2) / 720.0f);

	std::shared_ptr<Firefly::Instance> m_Instance = nullptr;
	std::shared_ptr<Firefly::GraphicsEngine> m_GraphicsEngine = nullptr;
	std::shared_ptr<Firefly::RenderTarget> m_RenderTarget = nullptr;

	std::shared_ptr<Firefly::Shader> m_VertexShader = nullptr;
	std::shared_ptr<Firefly::Shader> m_FragmentShader = nullptr;
	std::shared_ptr<Firefly::GraphicsPipeline> m_Pipeline = nullptr;

	std::shared_ptr<Firefly::Buffer> m_VertexBuffer = nullptr;
	std::shared_ptr<Firefly::Buffer> m_IndexBuffer = nullptr;

	std::shared_ptr<Firefly::Buffer> m_LeftEyeUniform = nullptr;
	std::shared_ptr<Firefly::Buffer> m_RightEyeUniform = nullptr;

	std::shared_ptr<Firefly::Buffer> m_UniformBuffer = nullptr;
	std::shared_ptr<Firefly::Image> m_Texture = nullptr;

	std::shared_ptr<Firefly::Package> m_VertexResourcePackageLeft = nullptr;
	std::shared_ptr<Firefly::Package> m_VertexResourcePackageRight = nullptr;
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

	Firefly::StereoCamera& getCamera() { return m_Camera; }
	void captureFrame() { m_bShouldCapture = true; }

private:
	std::vector<Vertex> generateTriangleVertices() const;
	std::vector<uint32_t> generateTriangleIndices() const;

	std::vector<Vertex> generateQuadVertices() const;
	std::vector<uint32_t> generateQuadIndices() const;

	std::vector<Vertex> generateCubeVertices() const;
	std::vector<uint32_t> generateCubeIndices() const;
};