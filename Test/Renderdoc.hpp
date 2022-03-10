#pragma once

#include "ThirdParty/renderdoc/renderdoc_app.h"

struct Renderdoc
{
	Renderdoc();
	~Renderdoc();

	void beginCapture();
	void endCapture();

	RENDERDOC_API_1_4_2* m_pRenderdocApp = nullptr;
	void* m_LibraryHandle = nullptr;
};