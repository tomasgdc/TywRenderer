//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "VKRenderer.h"


#if defined (WIN32) || defined(_WIN32) //START OF WINDOWS


bool GetModeListForDisplay(std::vector<vidMode> & modeList) 
{

	modeList.clear();
	int mode, displayNum;
	bool success = false;

	mode = 0;
	displayNum = 0;

	//first call gives graphic card's name "EnumDisplayDevices"
	//second call gives monitor's name "EnumDisplayDevices"
	DISPLAY_DEVICE	device;
	::ZeroMemory(&device, sizeof(DISPLAY_DEVICE));
	device.cb = sizeof(device);
	if (!EnumDisplayDevices(nullptr, displayNum, &device, 0)) {
		//Engine::getInstance().Sys_Error("ERROR: Could not get graphic card's name \r\n");
		return false;
	}

	// get the monitor for this display
	if (!(device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
		//Engine::getInstance().Sys_Error("ERROR: Could not get monitor for this display \r\n");
	}

	DISPLAY_DEVICE	monitor;
	::ZeroMemory(&monitor, sizeof(DISPLAY_DEVICE));
	monitor.cb = sizeof(monitor);
	if (!EnumDisplayDevices(device.DeviceName, 0, &monitor, 0)) {
		//Engine::getInstance().Sys_Error("ERROR: Could not get monitors name  \r\n");
		return false;
	}


	//get available monitor display resolutions
	do {
		DEVMODE dm;
		::ZeroMemory(&dm, sizeof(DEVMODE));
		dm.dmSize = sizeof(DEVMODE);
		success = EnumDisplaySettings(device.DeviceName, mode, &dm);
		mode++;

		if (!success) {
			break;
		}

		if (dm.dmBitsPerPel != 32) {
			continue;
		}
		if (dm.dmDisplayFrequency != 60) {
			continue;
		}


		vidMode mode_t;
		mode_t.iHeight = dm.dmPelsHeight;
		mode_t.iWidth = dm.dmPelsWidth;
		mode_t.iDisplayHz = dm.dmDisplayFrequency;

		modeList.push_back(mode_t);
	} while (success);
	return true;
}




const wchar_t* GetDeviceName(const int deviceNum)
{
	DISPLAY_DEVICE	device = {};
	device.cb = sizeof(device);
	if (!EnumDisplayDevices(
		0,			// lpDevice
		deviceNum,
		&device,
		0 /* dwFlags */)) {
		return false;
	}

	// get the monitor for this display
	if (!(device.StateFlags & DISPLAY_DEVICE_ATTACHED_TO_DESKTOP)) {
		return false;
	}
	return device.DeviceName;
}

bool GetDisplayCoordinates(const int deviceNum, int & x, int & y, int & width, int & height, int & displayHz)
{

	const wchar_t* deviceName = GetDeviceName(deviceNum);
	DISPLAY_DEVICE device = {};
	device.cb = sizeof(device);
	if (!EnumDisplayDevices(
		0,
		deviceNum,
		&device,
		0)) {
		return false;
	}

	DISPLAY_DEVICE monitor;
	monitor.cb = sizeof(monitor);
	if (!EnumDisplayDevices(
		deviceName,
		0,
		&monitor,
		0)) {
		return false;
	}

	DEVMODE devmode;
	devmode.dmSize = sizeof(devmode);
	if (!EnumDisplaySettings(deviceName, ENUM_CURRENT_SETTINGS, &devmode)) {
		return false;
	}

	x = devmode.dmPosition.x;
	y = devmode.dmPosition.y;
	width = devmode.dmPelsWidth;
	height = devmode.dmPelsHeight;
	displayHz = devmode.dmDisplayFrequency;
	return true;
}


bool GetWindowDimensions(const gl_params parms, int &x, int &y, int &w, int &h)
{
	//
	// compute width and height
	//
	if (parms.iFullScreen != 0)
	{
		if (parms.iFullScreen == -1)
		{
			// borderless window at specific location, as for spanning
			// multiple monitor outputs
			x = parms.iXCoord;
			y = parms.iYCoord;
			w = parms.iWidth;
			h = parms.iHeight;
		}
		else
		{
			// get the current monitor position and size on the desktop, assuming
			// any required ChangeDisplaySettings has already been done
			int displayHz = 0;
			if (!GetDisplayCoordinates(parms.iFullScreen - 1, x, y, w, h, displayHz))
			{
				return false;
			}
		}
	}
	else {
		RECT	r;

		// adjust width and height for window border
		r.bottom = parms.iHeight;
		r.left = 0;
		r.top = 0;
		r.right = parms.iWidth;

		AdjustWindowRect(&r, WINDOW_STYLE | WS_SYSMENU, FALSE);

		w = r.right - r.left;
		h = r.bottom - r.top;

		x = parms.iXCoord;
		y = parms.iYCoord;
	}

	return true;
}


#endif //END OF WINDOWS