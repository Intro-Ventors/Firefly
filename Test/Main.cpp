#include <iostream>

#define FIREFLY_SETUP_THIRD_PARTY
#include "Firefly/Instance.hpp"
#include "Firefly/Encoder/Encoder.hpp"
#include "Firefly/Decoder/Decoder.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"
#include "Firefly/Buffer.hpp"

int main()
{
	try
	{
		auto pInstance = Firefly::Instance::create(true, VK_API_VERSION_1_3);
		auto pEncoder = Firefly::Encoder::create(pInstance);
		auto pDecoder = Firefly::Decoder::create(pInstance);
		auto pGraphics = Firefly::GraphicsEngine::create(pInstance);
		auto pBuffer = Firefly::Buffer::create(pGraphics, 1024, Firefly::BufferType::Staging);

		auto ptr = pBuffer->mapMemory();
	}
	catch (const Firefly::BackendError& e)
	{
		std::cout << e.what();
	}

	return 0;
}