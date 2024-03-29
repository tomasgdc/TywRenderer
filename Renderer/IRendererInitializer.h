/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once



//forward declaration
struct gl_params;


struct  vidMode
{
	uint32_t iWidth;
	uint32_t iHeight;
	uint32_t iDisplayHz;
	
	bool operator==(const vidMode & a) 
	{
		return a.iWidth == iWidth && a.iHeight == iHeight && a.iDisplayHz == iDisplayHz;
	}
};


/*
	Used for OS handling
*/
class IRendererInitializer
{
public:
	virtual ~IRendererInitializer() {}

	virtual bool CreateRendererScreen(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT (CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM)) = 0;
	virtual void DestroyRendererScreen() = 0;
	virtual void RendererSwapBuffers() = 0;

#if defined (WIN32) || defined(_WIN32) //Start of Windows
	virtual HWND GetWind32Handle() = 0;
#endif

};


#if defined (WIN32) || defined(_WIN32) //Start of Windows

 extern bool GetModeListForDisplay(std::vector<vidMode> & modeList);
 extern const wchar_t* GetDeviceName(const int deviceNum);
 extern bool GetWindowDimensions(const gl_params parms, int &x, int &y, int &w, int &h);
 extern bool GetDisplayCoordinates(const int deviceNum, int & x, int & y, int & width, int & height, int & displayHz);

#endif //End of Windows