#define FIREFLY_SETUP_THIRD_PARTY
#include "TestEngine.hpp"

TestEngine::TestEngine()
{
	m_Instance = Firefly::Instance::create(true, VK_API_VERSION_1_1);
	m_GraphicsEngine = Firefly::GraphicsEngine::create(m_Instance);
	m_RenderTarget = Firefly::RenderTarget::create(m_GraphicsEngine, { 1280, 720, 1 }, 1);

	m_VertexShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.vert.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
	m_FragmentShader = Firefly::Shader::create(m_GraphicsEngine, "Shaders/shader.frag.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT);
	m_Pipeline = Firefly::GraphicsPipeline::create(m_GraphicsEngine, "Basic_Pipeline", { m_VertexShader, m_FragmentShader }, m_RenderTarget);

	{
		const auto vertices = generateVertices();
		auto pStagingVertexBuffer = Firefly::Buffer::create(m_GraphicsEngine, vertices.size() * sizeof(Vertex), Firefly::BufferType::Staging);

		std::copy(vertices.begin(), vertices.end(), reinterpret_cast<Vertex*>(pStagingVertexBuffer->mapMemory()));
		pStagingVertexBuffer->unmapMemory();

		m_VertexBuffer = Firefly::Buffer::create(m_GraphicsEngine, vertices.size() * sizeof(Vertex), Firefly::BufferType::Vertex);
		m_VertexBuffer->fromBuffer(pStagingVertexBuffer.get());
	}

	{
		const auto indices = generateIndices();
		auto pStagingIndexBuffer = Firefly::Buffer::create(m_GraphicsEngine, indices.size() * sizeof(uint32_t), Firefly::BufferType::Staging);

		std::copy(indices.begin(), indices.end(), reinterpret_cast<uint32_t*>(pStagingIndexBuffer->mapMemory()));
		pStagingIndexBuffer->unmapMemory();

		m_IndexBuffer = Firefly::Buffer::create(m_GraphicsEngine, indices.size() * sizeof(uint32_t), Firefly::BufferType::Index);
		m_IndexBuffer->fromBuffer(pStagingIndexBuffer.get());
	}
}

std::shared_ptr<Firefly::Image> TestEngine::draw() const
{
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
		m_Pipeline->bind(pCommandBuffer);

		pCommandBuffer->bindScissor(scissor);
		pCommandBuffer->bindViewport(viewport);
		pCommandBuffer->drawIndices(3);
	}

	// Right eye.
	{
		m_VertexBuffer->bindAsVertexBuffer(pCommandBuffer);
		m_IndexBuffer->bindAsIndexBuffer(pCommandBuffer);
		m_Pipeline->bind(pCommandBuffer);

		viewport.x = viewport.width;
		pCommandBuffer->bindViewport(viewport);
		pCommandBuffer->bindScissor(scissor);
		pCommandBuffer->drawIndices(3);
	}

	m_RenderTarget->submitFrame();

	return m_RenderTarget->getColorAttachment();
}

std::vector<TestEngine::Vertex> TestEngine::generateVertices() const
{
	return {
		{{0.0f, -0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
		{{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
	};
}

std::vector<uint32_t> TestEngine::generateIndices() const
{
	return { 0, 1, 2 };
}
