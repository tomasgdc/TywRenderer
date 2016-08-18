/* ============================================================================
* Tywyl Engine
* Platform:      Windows
* WWW:
* ----------------------------------------------------------------------------
* Copyright 2015 Tomas Mikalauskas. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice,
*     this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright
*     notice, this list of conditions and the following disclaimer in the
*     documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY TOMAS MIKALAUSKAS ''AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
* EVENT SHALL TOMAS MIKALAUSKAS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
* INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
* THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
* The views and conclusions contained in the software and documentation are
* those of the authors and should not be interpreted as representing official
* policies, either expressed or implied, of Tomas Mikalauskas.

DISCLAIMER
The engine design is based on Doom3 BFG engine
https://github.com/id-Software/DOOM-3-BFG.
A lot of tweaks maded to suit my needs.
Tywyll game engine design and functionality will be changed with time.
============================================================================
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


//Engine Export
#ifdef TywEngine_EXPORTS
	#define TYWENGINE_API DLL_EXPORT
#else
	#define TYWENGINE_API DLL_IMPORT
#endif

//Animation Export
#ifdef TywAnimation_EXPORTS
	#define TYWANIMATION_API DLL_EXPORT
#else
	#define TYWANIMATION_API DLL_IMPORT
#endif

//Game Export
#ifdef TywGame_EXPORTS
	#define TYWGAME_API DLL_EXPORT
#else
	#define TYWGAME_API DLL_IMPORT
#endif


//Physics Export
#ifdef TywPhysics_EXPORTS
	#define TYWPHYSICS_API DLL_EXPORT
#else
	#define TYWPHYSICS_API DLL_IMPORT
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

#define TEXTURE_LOCATION	"bin/Data/textures/"
#define GEOMETRY_LOCATION	"bin/Data/geometry/"
#define MATERIAL_LOCATION	"bin/Data/material/"
#define ANIMATION_LOCATION	"bin/Data/animation/"
