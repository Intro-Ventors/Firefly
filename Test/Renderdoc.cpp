#include "Renderdoc.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <stdexcept>

Renderdoc::Renderdoc()
{
	const auto pHandle = LoadLibraryA("renderdoc.dll");
	pRENDERDOC_GetAPI RENDERDOC_GetAPI = reinterpret_cast<pRENDERDOC_GetAPI>(GetProcAddress(pHandle, "RENDERDOC_GetAPI"));

	if (RENDERDOC_GetAPI(eRENDERDOC_API_Version_1_4_2, (void**)&m_pRenderdocApp) != 1)
		throw std::runtime_error("Failed to setup RenderDoc!");

	m_LibraryHandle = pHandle;
}

Renderdoc::~Renderdoc()
{
	FreeLibrary(static_cast<HMODULE>(m_LibraryHandle));
}

void Renderdoc::beginCapture()
{
	if (m_pRenderdocApp)
		m_pRenderdocApp->StartFrameCapture(NULL, NULL);
}

void Renderdoc::endCapture()
{
	if (m_pRenderdocApp)
		m_pRenderdocApp->EndFrameCapture(NULL, NULL);
}
