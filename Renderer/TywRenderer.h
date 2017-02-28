#pragma once

namespace tywrnd
{
	/*
		Initialize
	*/
	bool initialize(uint32_t height, uint32_t width, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM));


	/*
		Submit pipeline
	*/
	void submitPipeline();


	/*
		Render
	*/
	void frame();
}