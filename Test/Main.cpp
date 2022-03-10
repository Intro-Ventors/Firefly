#include <iostream>

#include "TestEngine.hpp"
#include "ThirdParty/lodepng/lodepng.h"

void SaveImage(const std::shared_ptr<Firefly::Image>& pImage)
{
	const auto pBuffer = pImage->toBuffer();
	unsigned char* outputData = nullptr;
	size_t outputSize = 0;

	// Encode the image to PNG.
	if (lodepng_encode_memory(&outputData, &outputSize, reinterpret_cast<unsigned char*>(pBuffer->mapMemory()), pImage->getExtent().width, pImage->getExtent().height, LCT_RGBA, 8))
		throw std::runtime_error("Failed to encode the image to PNG!");

	std::fstream imageFile("Scene.png", std::ios::out | std::ios::binary);

	if (!imageFile.is_open())
		throw std::runtime_error("Failed to open the image file!");

	imageFile.write(reinterpret_cast<char*>(outputData), outputSize);
	imageFile.close();
}

int main()
{
	try
	{
		auto engine = TestEngine();
		SaveImage(engine.draw());
	}
	catch (const Firefly::BackendError& e)
	{
		std::cout << "Backend error: " << e.what() << std::endl;
	}
	catch (const std::runtime_error& e)
	{
		std::cout << "Runtime error: " << e.what() << std::endl;
	}

	return 0;
}