#include <iostream>

#include "TestEngine.hpp"
#include "ThirdParty/lodepng/lodepng.h"

#include <chrono>
#include <fstream>

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
		using Clock = std::chrono::high_resolution_clock;
		auto oldTimePoint = Clock::now();

		bool shouldRun = true;
		while (shouldRun)
		{
			const auto currentTime = Clock::now();
			const auto difference = currentTime - oldTimePoint;

			std::cout << std::chrono::duration_cast<std::chrono::microseconds>(difference) << std::endl;
			auto image = engine.draw();

			for (const auto input : std::move(engine.getSurface()->getKeyInputs()))
			{
				switch (input.getKey())
				{
				case Firefly::Key::W:
					if (input.isPressed())
					{
						engine.getCamera().moveForward(difference.count());
						std::cout << "Camera moved forward.\n";
					}
					break;

				case Firefly::Key::S:
					if (input.isPressed())
					{
						engine.getCamera().moveBackward(difference.count());
						std::cout << "Camera moved backwards.\n";
					}
					break;

				case Firefly::Key::A:
					if (input.isPressed())
					{
						engine.getCamera().moveLeft(difference.count());
						std::cout << "Camera moved to the left.\n";
					}
					break;

				case Firefly::Key::D:
					if (input.isPressed())
					{
						engine.getCamera().moveRight(difference.count());
						std::cout << "Camera moved to the right.\n";
					}
					break;

				case Firefly::Key::Space:
					if (input.isPressed() && input.shiftPressed())
					{
						engine.getCamera().moveUp(difference.count());
						std::cout << "Camera moved up.\n";
					}
					else
					{
						engine.getCamera().moveDown(difference.count());
						std::cout << "Camera moved down.\n";
					}
					break;

				case Firefly::Key::R:
					if (input.isPressed())
					{
						engine.getCamera().m_EyeSeparation += 0.0001f;
						std::cout << "Eye separation updated: " << engine.getCamera().m_EyeSeparation << std::endl;
					}
					break;

				case Firefly::Key::T:
					if (input.isPressed())
					{
						engine.getCamera().m_EyeSeparation -= 0.0001f;
						std::cout << "Eye separation updated: " << engine.getCamera().m_EyeSeparation << std::endl;
					}
					break;

				case Firefly::Key::C:
					if (input.isPressed())
					{
						engine.captureFrame();
						std::cout << "Capturing set for this frame.\n";
					}
					break;

				case Firefly::Key::F:
					if (input.isPressed())
					{
						SaveImage(image);
						std::cout << "Image saved.\n";
					}
					break;

				case Firefly::Key::X:
					if (input.isPressed())
					{
						shouldRun = false;
					}
					break;
				}
			}

			oldTimePoint = currentTime;
		}
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