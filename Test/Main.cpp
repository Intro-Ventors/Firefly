#include <iostream>

#include "GraphicsCore/Instance.hpp"
#include "GraphicsCore/Encoder/Encoder.hpp"
#include "GraphicsCore/Decoder/Decoder.hpp"

int main()
{
	auto pInstance = GraphicsCore::Instance::create(true);
	auto pEncoder = GraphicsCore::Encoder::create(pInstance);
	auto pDecoder = GraphicsCore::Decoder::create(pInstance);

	return 0;
}