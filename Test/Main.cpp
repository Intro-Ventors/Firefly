#include <iostream>

#include "GraphicsCore/Instance.hpp"
#include "GraphicsCore/Encoder/Encoder.hpp"
#include "GraphicsCore/Decoder/Decoder.hpp"

int main()
{
	auto pInstance = GraphicsCore::Instance::create(true);
	auto pEncoder = pInstance->createEncoder();
	auto pDecoder = pInstance->createDecoder();

	return 0;
}