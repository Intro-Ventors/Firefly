#include <iostream>

#define FIREFLY_SETUP_THIRD_PARTY
#include "Firefly/Instance.hpp"
#include "Firefly/Encoder/Encoder.hpp"
#include "Firefly/Decoder/Decoder.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Buffer.hpp"
#include "Firefly/Image.hpp"

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
	}
	catch (const Firefly::BackendError& e)
	{
		std::cout << e.what();
	}

	return 0;
}