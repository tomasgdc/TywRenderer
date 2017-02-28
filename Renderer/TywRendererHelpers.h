#pragma once
#include <RendererPch\macro.h>

#include <cstdarg> //http://www.cplusplus.com/reference/cstdarg/va_start/
#include <cstdint> //http://en.cppreference.com/w/cpp/header/cstdint


namespace tywrnd
{
	/*
		
	*/
	void log(const char* _filePath, uint16_t _line, const char* _format, va_list _argList);

	/*
		
	*/
	void debugOutput(const char* _out);
}


#define TYW_TRACE _TYW_TRACE
#define _TYW_TRACE(_format, ...) \
				TYW_MACRO_BLOCK_BEGIN \
					tywrnd::log(__FILE__, uint16_t(__LINE__), "TywRenderer " _format "\n", ##__VA_ARGS__); \
				TYW_MACRO_BLOCK_END


namespace tywrnd
{
	/*

	*/
	void memCopy(void* _dst, const void* _src, size_t _numBytes);
}