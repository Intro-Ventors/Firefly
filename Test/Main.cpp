#include <iostream>

#define FIREFLY_SETUP_THIRD_PARTY
#include "Firefly/Instance.hpp"
#include "Firefly/Encoder/Encoder.hpp"
#include "Firefly/Decoder/Decoder.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Buffer.hpp"
#include "Firefly/Image.hpp"
#include "Firefly/Shader.hpp"
#include "Firefly/Graphics/RenderTarget.hpp"

int main()
{
	try
	{
		auto pInstance = Firefly::Instance::create(true, VK_API_VERSION_1_1);
		auto pEncoder = Firefly::Encoder::create(pInstance);
		auto pDecoder = Firefly::Decoder::create(pInstance);
		auto pGraphics = Firefly::GraphicsEngine::create(pInstance);
		auto pBuffer = Firefly::Buffer::create(pGraphics, 1024, Firefly::BufferType::Staging);

		auto ptr = pBuffer->mapMemory();

		auto pImage = Firefly::Image::create(pGraphics, { 512, 512, 1 }, VkFormat::VK_FORMAT_B8G8R8A8_SRGB, Firefly::ImageType::TwoDimension, 1);
		auto pCopyBuffer = pImage->toBuffer();

		auto pShader = Firefly::Shader::create(pGraphics, "E:\\Dynamik\\Game Repository\\assets\\assets\\Cube\\vert.spv", VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT);
		pShader->terminate();

		auto pRenderTarget = Firefly::RenderTarget::create(pGraphics, { 512, 512, 1 }, 1);
		pRenderTarget->terminate();
	}
	catch (const Firefly::BackendError& e)
	{
		std::cout << e.what();
	}

	return 0;
}