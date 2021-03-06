#pragma once

#include <renderdoc/renderdoc_app.h>

namespace Firefly
{
	/**
	 * Renderdoc class.
	 * This tool is used to relay information to the renderdoc debugging tool.
	 *
	 * When needed, start by calling the beginCapture() method and make sure to follow it with endCapture() when you want to finish the renderdoc capture.
	 *
	 * Required shared libraries:
	 * - Windows:	renderdoc.dll
	 * - Linux:		librenderdoc.so
	 * - Android:	libVkLayer_GLES_RenderDoc.so
	 *
	 * Note that only one instance of this object is needed for the whole application.
	 */
	class Renderdoc final
	{
	public:
		/**
		 * Default constructor.
		 */
		Renderdoc();

		/**
		 * Default destructor.
		 */
		~Renderdoc();

		/**
		 * Begin the capture.
		 */
		void beginCapture() const;

		/**
		 * End the capture.
		 */
		void endCapture() const;

	private:
		RENDERDOC_API_1_4_2* m_pRenderdocApp = nullptr;
		void* m_LibraryHandle = nullptr;
	};
}