/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include <RendererPch\macro.h>

#include <cstdarg> //http://www.cplusplus.com/reference/cstdarg/va_start/
#include <cstdint> //http://en.cppreference.com/w/cpp/header/cstdint


namespace vkfx
{
	/*
		
	*/
	void log(const char* _filePath, uint16_t _line, const char* _format, va_list _argList);

	/*
		
	*/
	void debugOutput(const char* _out);
}


#define VKFX_TRACE _VKFX_TRACE
#define _VKFX_TRACE(_format, ...) \
				VKFX_MACRO_BLOCK_BEGIN \
					vkfx::log(__FILE__, uint16_t(__LINE__), "vkfx " _format "\n", ##__VA_ARGS__); \
				VKFX_MACRO_BLOCK_END


namespace vkfx
{
	/*

	*/
	void memCopy(void* _dst, const void* _src, size_t _numBytes);
}