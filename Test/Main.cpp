#include <iostream>

#include "TestEngine.hpp"
#include "ThirdParty/lodepng/lodepng.h"

#include <chrono>
#include <fstream>

#ifdef FIREFLY_PLATFORM_WINDOWS

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/highgui.hpp>

#endif

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

			if (GetKeyState('W') < 0)
			{
				engine.getCamera().moveForward(difference.count());
				std::cout << "Camera moved forward.\n";
			}
			else if (GetKeyState('S') < 0)
			{
				engine.getCamera().moveBackward(difference.count());
				std::cout << "Camera moved backwards.\n";
			}

			if (GetKeyState('A') < 0)
			{
				engine.getCamera().moveLeft(difference.count());
				std::cout << "Camera moved to the left.\n";
			}
			else if (GetKeyState('D') < 0)
			{
				engine.getCamera().moveRight(difference.count());
				std::cout << "Camera moved to the right.\n";
			}

			if (GetKeyState(' ') < 0)
			{
				if (GetKeyState(VK_LSHIFT) < 0)
				{
					engine.getCamera().moveDown(difference.count());
					std::cout << "Camera moved down.\n";
				}
				else
				{
					engine.getCamera().moveUp(difference.count());
					std::cout << "Camera moved up.\n";
				}
			}

			if (GetKeyState('R') < 0)
			{
				engine.getCamera().m_EyeSeperation += 0.0001f;
				std::cout << "Eye separation updated: " << engine.getCamera().m_EyeSeperation << std::endl;
			}
			else if (GetKeyState('T') < 0)
			{
				engine.getCamera().m_EyeSeperation -= 0.0001f;
				std::cout << "Eye separation updated: " << engine.getCamera().m_EyeSeperation << std::endl;
			}

			auto image = engine.draw();

			if (GetKeyState('C') < 0)
			{
				engine.captureFrame();
				std::cout << "Capturing set for this frame.\n";
			}
			else if (GetKeyState('F') < 0)
			{
				SaveImage(image);
				std::cout << "Image saved.\n";
			}

			if (GetKeyState('X') < 0)
			{
				shouldRun = false;
			}

			const auto pBuffer = image->toBuffer();
			cv::Mat imageMat = cv::Mat(image->getExtent().height, image->getExtent().width, CV_8UC4, pBuffer->mapMemory());
			pBuffer->unmapMemory();

			cv::pollKey();
			cv::imshow("Firefly", imageMat);

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