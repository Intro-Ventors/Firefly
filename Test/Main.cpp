#include <iostream>

#include "Firefly/Instance.hpp"
#include "Firefly/Encoder/Encoder.hpp"
#include "Firefly/Decoder/Decoder.hpp"
#include "Firefly/Graphics/GraphicsEngine.hpp"

int main()
{
	auto pInstance = Firefly::Instance::create(true, VK_API_VERSION_1_1);
	auto pEncoder = Firefly::Encoder::create(pInstance);
	auto pDecoder = Firefly::Decoder::create(pInstance);
	auto pGraphics = Firefly::GraphicsEngine::create(pInstance);

	return 0;
}