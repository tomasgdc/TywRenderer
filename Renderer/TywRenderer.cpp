//stdafx
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "TywRendererHelpers.h"
#include "TywRenderer.h"

//Vulkan RendererIncludes
#include "VKRenderer.h"


//Other Includes
#include <cstdio>  //http://www.cplusplus.com/reference/cstdio/vfprintf/
#include <cstdint> //http://en.cppreference.com/w/cpp/header/cstdint
#include <cstring> //http://en.cppreference.com/w/cpp/string/byte/memcpy
#include <memory>  //smart pointer


static std::unique_ptr<IRenderer>			 g_pRenderer = nullptr;
static bool									 g_bInitialized = false;

namespace tywrnd
{
	bool initialize(uint32_t height, uint32_t width, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM))
	{
		if (g_bInitialized)
		{
			return true;
		}
		g_pRenderer = std::make_unique<VKRenderer>();

		TYW_TRACE("Initializing \n", "%s");
		g_bInitialized = g_pRenderer->VInitRenderer(height, width, isFullscreen, MainWindowProc);
		if (!g_bInitialized)
		{

		}
		return g_bInitialized;
	}



	void submitPipeline()
	{

	}



	void frame()
	{

	}


	void log(const char* _filePath, uint16_t _line, const char* _format, va_list _argList)
	{
		char temp[2048];
		char* out = temp;
		va_list argListCopy;
		va_copy(argListCopy, _argList);
		int32_t len = snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
		int32_t total = len + vsnprintf(out + len, sizeof(temp) - len, _format, argListCopy);
		va_end(argListCopy);
		if ((int32_t)sizeof(temp) < total)
		{
			out = (char*)alloca(total + 1);
			tywrnd::memCopy(out, temp, len);
			vsnprintf(out + len, total - len, _format, _argList);
		}
		out[total] = '\0';
		tywrnd::debugOutput(out);
	}

	void debugOutput(const char* _out)
	{
		fputs(_out, stdout);
		fflush(stdout);
	}

	void memCopy(void* _dst, const void* _src, size_t _numBytes)
	{
		::memcpy(_dst, _src, _numBytes);
	}
}