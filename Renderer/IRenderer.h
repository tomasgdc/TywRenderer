/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
class  IRenderState
{
public:
	virtual std::string VToString() = 0;
};



class  IRenderer
{
public:
	virtual ~IRenderer(){}

	virtual bool VInitRenderer(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM)) = 0;
	virtual void VSetBackgroundColor(BYTE bgA, BYTE bgR, BYTE bgG, BYTE bgB) = 0;
	virtual void VWindowResize(uint32_t iHeight, uint32_t iWidth) = 0;
	virtual void VShutdown() = 0;
	virtual void StartFrame() = 0;
	virtual void EndFrame(uint64_t* gpuMicroSec) = 0;
};

