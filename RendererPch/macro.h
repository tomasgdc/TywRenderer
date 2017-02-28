/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once

//maps a base version of the CRT heap functions to the corresponding debug version.
#define _CRTDBG_MAP_ALLOC

//Exporintg DLL
//===================================================================================
#if defined(__GNUC__)
	#define DLL_EXPORT __attribute__ ((visibility("default")))
	#define DLL_IMPORT __attribute__ ((visibility("default")))
#else
	#define DLL_EXPORT __declspec(dllexport)
	#define DLL_IMPORT __declspec(dllimport)
#endif


//Renderer Export
#ifdef TywRenderer_EXPORTS
	#define TYWRENDERER_API DLL_EXPORT
#else
	#define TYWRENDERER_API DLL_IMPORT
#endif
//===================================================================================


#ifndef BIT
#define BIT( num )				( 1ULL << ( num ) )
#endif

#define SAFE_DELETE(x) if(x) {delete x; x=nullptr;}
#define SAFE_DELETE_ARRAY(x) if (x) { delete [] x; x=nullptr; }
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#define _TEXT(x) #x

#if defined (_DEBUG) || defined (DEBUG)
#	define TYW_NEW new(_NORMAL_BLOCK,__FILE__, __LINE__)
#else
#	define TYW_NEW new
#endif

#define TYW_MACRO_BLOCK_BEGIN for(;;) {
#define TYW_MACRO_BLOCK_END break; }

#define TEXTURE_LOCATION	"bin/Data/textures/"
#define GEOMETRY_LOCATION	"bin/Data/geometry/"
#define MATERIAL_LOCATION	"bin/Data/material/"
#define ANIMATION_LOCATION	"bin/Data/animation/"
