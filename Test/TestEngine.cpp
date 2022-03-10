#define FIREFLY_SETUP_THIRD_PARTY
#include "TestEngine.hpp"

#include "Firefly/AssetsLoaders/ImageLoader.hpp"
#include "Firefly/AssetsLoaders/ObjLoader.hpp"

TestEngine::TestEngine()
{
	m_Instance = Firefly::Instance::create(true, VK_API_VERSION_1_1);
	m_GraphicsEngine = Firefly::GraphicsEngine::create(m_Instance);
	m_RenderTarget = Firefly::RenderTarget::create(m_GraphicsEngine, { 1280, 720, 1 }, 1);

	m_VertexShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.vert.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	m_FragmentShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.frag.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	m_Pipeline = Firefly::GraphicsPipeline::create(m_GraphicsEngine, "Basic_Pipeline", { m_VertexShader, m_FragmentShader }, m_RenderTarget);
	m_VertexResourcePackage = m_Pipeline->createPackage(m_VertexShader.get());
	m_FragmentResourcePackage = m_Pipeline->createPackage(m_FragmentShader.get());

	//{
	//	const auto vertices = generateQuadVertices();
	//	m_VertexCount = static_cast<uint32_t>(vertices.size());
	//	auto pStagingVertexBuffer = Firefly::Buffer::create(m_GraphicsEngine, m_VertexCount * sizeof(Vertex), Firefly::BufferType::Staging);
	//
	//	std::copy(vertices.begin(), vertices.end(), reinterpret_cast<Vertex*>(pStagingVertexBuffer->mapMemory()));
	//	pStagingVertexBuffer->unmapMemory();
	//
	//	m_VertexBuffer = Firefly::Buffer::create(m_GraphicsEngine, m_VertexCount * sizeof(Vertex), Firefly::BufferType::Vertex);
	//	m_VertexBuffer->fromBuffer(pStagingVertexBuffer.get());
	//}
	//
	//{
	//	const auto indices = generateQuadIndices();
	//	m_IndexCount = static_cast<uint32_t>(indices.size());
	//	auto pStagingIndexBuffer = Firefly::Buffer::create(m_GraphicsEngine, m_IndexCount * sizeof(uint32_t), Firefly::BufferType::Staging);
	//
	//	std::copy(indices.begin(), indices.end(), reinterpret_cast<uint32_t*>(pStagingIndexBuffer->mapMemory()));
	//	pStagingIndexBuffer->unmapMemory();
	//
	//	m_IndexBuffer = Firefly::Buffer::create(m_GraphicsEngine, m_IndexCount * sizeof(uint32_t), Firefly::BufferType::Index);
	//	m_IndexBuffer->fromBuffer(pStagingIndexBuffer.get());
	//}

	{
		auto model = Firefly::LoadObjModel(m_GraphicsEngine, "Assets/VikingRoom/untitled.obj");

		m_VertexBuffer = model.m_VertexBuffer;
		m_VertexCount = static_cast<uint32_t>(model.m_VertexCount);

		m_IndexBuffer = model.m_IndexBuffer;
		m_IndexCount = static_cast<uint32_t>(model.m_IndexCount);
	}

	m_UniformBuffer = Firefly::CameraMatrix::createBuffer(m_GraphicsEngine);
	m_VertexResourcePackage->bindResources(0, { m_UniformBuffer });

	m_Texture = Firefly::LoadImage(m_GraphicsEngine, "Assets/VikingRoom/texture.png");
	m_FragmentResourcePackage->bindResources(0, { m_Texture });
}

std::shared_ptr<Firefly::Image> TestEngine::draw()
{
	if (m_bShouldCapture)
		m_RenderdocIntegration.beginCapture();

	m_Camera.update();
	m_Camera.copyToBuffer(m_UniformBuffer.get());

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
	{
		m_VertexBuffer->bindAsVertexBuffer(pCommandBuffer);
		m_IndexBuffer->bindAsIndexBuffer(pCommandBuffer);
		m_Pipeline->bind(pCommandBuffer, { m_VertexResourcePackage.get(), m_FragmentResourcePackage.get() });

		pCommandBuffer->bindScissor(scissor);
		pCommandBuffer->bindViewport(viewport);
		pCommandBuffer->drawIndices(m_IndexCount);
	}

	// Right eye.
	{
		m_VertexBuffer->bindAsVertexBuffer(pCommandBuffer);
		m_IndexBuffer->bindAsIndexBuffer(pCommandBuffer);
		m_Pipeline->bind(pCommandBuffer, { m_VertexResourcePackage.get(), m_FragmentResourcePackage.get() });

		viewport.x = viewport.width;
		pCommandBuffer->bindViewport(viewport);
		pCommandBuffer->bindScissor(scissor);
		pCommandBuffer->drawIndices(m_IndexCount);
	}

	m_RenderTarget->submitFrame();

	if (m_bShouldCapture)
	{
		m_RenderdocIntegration.endCapture();
		m_bShouldCapture = false;
	}

	return m_RenderTarget->getColorAttachment();
}

std::vector<TestEngine::Vertex> TestEngine::generateVertices() const
{
	return {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{-0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f, 1.0f}, {0.0f, 0.0f}}
	};
}

std::vector<uint32_t> TestEngine::generateIndices() const
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
