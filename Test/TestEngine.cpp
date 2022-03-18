#define FIREFLY_ENABLE_LOGGING
#include "TestEngine.hpp"

#include "Firefly/AssetsLoaders/ImageLoader.hpp"
#include "Firefly/AssetsLoaders/ObjLoader.hpp"

#include "Firefly/ImportSourceFiles.hpp"

#include <fstream>

void logger(const Firefly::Utility::LogLevel level, const std::string_view& message)
{
	static std::ofstream outputFile("DebugOutput.txt");
	outputFile  << "[" << Firefly::Utility::LogLevelToString(level) << "] " << message << std::endl;
}

TestEngine::TestEngine()
{
	Firefly::Utility::Logger::setLoggerMethod(&logger);

	m_Instance = Firefly::Instance::create(VK_API_VERSION_1_1);
	m_GraphicsEngine = Firefly::GraphicsEngine::create(m_Instance);
	m_RenderTarget = Firefly::RenderTarget::create(m_GraphicsEngine, { 1280, 720, 1 }, VkFormat::VK_FORMAT_B8G8R8A8_SRGB, 1);
	m_Surface = Firefly::WindowsSurface::create(m_Instance, 1280, 720, L"Firefly");

	m_VertexShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.vert.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	m_FragmentShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.frag.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);

	m_Pipeline = Firefly::GraphicsPipeline::create(m_GraphicsEngine, "Basic_Pipeline", { m_VertexShader, m_FragmentShader }, m_RenderTarget);
	m_VertexResourcePackageLeft = m_Pipeline->createPackage(m_VertexShader.get());
	m_VertexResourcePackageRight = m_Pipeline->createPackage(m_VertexShader.get());
	m_FragmentResourcePackage = m_Pipeline->createPackage(m_FragmentShader.get());

	{
		auto model = Firefly::LoadObjModel(m_GraphicsEngine, "Assets/VikingRoom/untitled.obj");

		m_VertexBuffer = model.m_VertexBuffer;
		m_VertexCount = static_cast<uint32_t>(model.m_VertexCount);

		m_IndexBuffer = model.m_IndexBuffer;
		m_IndexCount = static_cast<uint32_t>(model.m_IndexCount);
	}

	m_UniformBuffer = Firefly::Buffer::create(m_GraphicsEngine, sizeof(glm::mat4), Firefly::BufferType::Uniform);
	auto& modelMatrix = *reinterpret_cast<glm::mat4*>(m_UniformBuffer->mapMemory());
	modelMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	m_UniformBuffer->unmapMemory();

	m_LeftEyeUniform = Firefly::CameraMatrix::createBuffer(m_GraphicsEngine);
	m_RightEyeUniform = Firefly::CameraMatrix::createBuffer(m_GraphicsEngine);

	m_VertexResourcePackageLeft->bindResources(0, { m_LeftEyeUniform });
	m_VertexResourcePackageLeft->bindResources(1, { m_UniformBuffer });
	m_VertexResourcePackageRight->bindResources(0, { m_RightEyeUniform });
	m_VertexResourcePackageRight->bindResources(1, { m_UniformBuffer });

	m_Texture = Firefly::LoadImageFromFile(m_GraphicsEngine, "Assets/VikingRoom/texture.png");
	m_FragmentResourcePackage->bindResources(0, { m_Texture });
}

std::shared_ptr<Firefly::Image> TestEngine::draw()
{
	if (m_bShouldCapture)
		m_RenderdocIntegration.beginCapture();

	m_Camera.update();
	m_Camera.copyToBuffer(m_LeftEyeUniform.get(), m_RightEyeUniform.get());

	VkViewport viewport = {};
	viewport.width = static_cast<float>(m_RenderTarget->getExtent().width) / 2;
	viewport.height = static_cast<float>(m_RenderTarget->getExtent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	viewport.x = 0.0f;
	viewport.y = 0.0f;

	VkRect2D scissor = {};
	scissor.extent.width = m_RenderTarget->getExtent().width;
	scissor.extent.height = m_RenderTarget->getExtent().height;
	scissor.offset.x = 0;
	scissor.offset.y = 0;

	const auto pCommandBuffer = m_RenderTarget->setupFrame(Firefly::CreateClearValues(Firefly::CreateColor256(0), Firefly::CreateColor256(0), Firefly::CreateColor256(0)));

	// Left eye.
	m_VertexBuffer->bindAsVertexBuffer(pCommandBuffer);
	m_IndexBuffer->bindAsIndexBuffer(pCommandBuffer);
	m_Pipeline->bind(pCommandBuffer, { m_VertexResourcePackageLeft.get(), m_FragmentResourcePackage.get() });

	pCommandBuffer->bindScissor(scissor);
	pCommandBuffer->bindViewport(viewport);
	pCommandBuffer->drawIndices(m_IndexCount);

	// Right eye.
	m_VertexBuffer->bindAsVertexBuffer(pCommandBuffer);
	m_IndexBuffer->bindAsIndexBuffer(pCommandBuffer);
	m_Pipeline->bind(pCommandBuffer, { m_VertexResourcePackageRight.get(), m_FragmentResourcePackage.get() });

	viewport.x = viewport.width;
	pCommandBuffer->bindViewport(viewport);
	pCommandBuffer->bindScissor(scissor);
	pCommandBuffer->drawIndices(m_IndexCount);

	m_RenderTarget->submitFrame();

	if (m_bShouldCapture)
	{
		m_RenderdocIntegration.endCapture();
		m_bShouldCapture = false;
	}

	return m_RenderTarget->getColorAttachment();
}

std::vector<TestEngine::Vertex> TestEngine::generateTriangleVertices() const
{
	return {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
	};
}

std::vector<uint32_t> TestEngine::generateTriangleIndices() const
{
	return { 0, 1, 2 };
}

std::vector<TestEngine::Vertex> TestEngine::generateQuadVertices() const
{
	return {
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
	};
}

std::vector<uint32_t> TestEngine::generateQuadIndices() const
{
	return {
		0, 1, 2,
		2, 3, 0
	};
}

std::vector<TestEngine::Vertex> TestEngine::generateCubeVertices() const
{
	return std::vector<Vertex>();
}

std::vector<uint32_t> TestEngine::generateCubeIndices() const
{
	return std::vector<uint32_t>();
}
