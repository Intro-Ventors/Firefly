#include "Firefly/Tools/Renderdoc.hpp"
#include "Firefly/BackendError.hpp"

#ifdef _WIN32
#	define WIN32_LEAN_AND_MEAN
#	include <Windows.h>

#else
#	include <dlfcn.h>

#endif

namespace Firefly
{
	Renderdoc::Renderdoc()
	{
#if defined(_WIN32)
		// Load the library.
		const auto pHandle = LoadLibrary(L"renderdoc.dll");

		// Validate the handle to check if we could load the library or not.
		if (!pHandle)
			throw BackendError("Failed to load the renderdoc dynamic library!");

		// Get the function pointer.
		const auto GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(pHandle, "RENDERDOC_GetAPI"));

#else
#	if defined(__APPLE__)
		// Load the library.
		const auto pHandle = dlopen("librenderdoc.dylib", RTLD_NOW | RTLD_LOCAL);

#	elif defined(__ANDROID__)
		// Load the library.
		const auto pHandle = dlopen("libVkLayer_GLES_RenderDoc.so", RTLD_NOW | RTLD_LOCAL);

#	else
		// Load the library.
		const auto pHandle = dlopen("librenderdoc.so", RTLD_NOW | RTLD_LOCAL);

#	endif

		// Validate the handle to check if we could load the library or not.
		if (!pHandle)
			throw BackendError("Failed to load the renderdoc dynamic library!");

		// Get the function pointer.
		const auto GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(dlsym(pHandle, "RENDERDOC_GetAPI"));

#endif

		// Validate and setup the renderdoc API.
		if (!GetAPI || GetAPI(eRENDERDOC_API_Version_1_4_2, reinterpret_cast<void**>(&m_pRenderdocApp)) != 1)
			throw BackendError("Failed to setup RenderDoc!");

		m_LibraryHandle = pHandle;
	}

	Renderdoc::~Renderdoc()
	{

#if defined(_WIN32)
		FreeLibrary(static_cast<HMODULE>(m_LibraryHandle));

#else
		dlclose(m_LibraryHandle);

#endif

	}

	void Renderdoc::beginCapture() const
	{
		m_pRenderdocApp->StartFrameCapture(nullptr, nullptr);
	}

	void Renderdoc::endCapture() const
	{
		m_pRenderdocApp->EndFrameCapture(nullptr, nullptr);
	}
}

#ifdef max
#undef max

#endif