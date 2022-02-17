#include <iostream>

#include "RCHAC/Core/Instance.hpp"
#include "RCHAC/Encoder/Encoder.hpp"
#include "RCHAC/Decoder/Decoder.hpp"

int main()
{
	auto pInstance = RCHAC::Instance::create(true);
	auto pEncoder = pInstance->createEncoder();
	auto pDecoder = pInstance->createDecoder();

	return 0;
}