#include <iostream>

#include "RCHAC/Core/Instance.hpp"
#include "RCHAC/Encoder/Encoder.hpp"

int main()
{
	auto pInstance = RCHAC::Instance::create(true);
	auto pEncoder = pInstance->createEncoder();

	return 0;
}