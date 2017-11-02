/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>


//Vulkan Includes
#include "Vulkan\VulkanTools.h"
#include "Vulkan\VulkanDebug.h"

//Renderer Includes
#include "VKRenderer.h"




std::string className = "window_class";
std::string titleName = "Vulkan demo";

VkPipelineCache VulkanRendererInitializer::m_PipelineCache;
VkCommandPool   VulkanRendererInitializer::m_CmdPool;
VkQueue		    VulkanRendererInitializer::m_Queue;

//In Vulkan the API version is encoded as a 32 - bit integer with the major and minor version being encoded into bits 31 - 22 and 21 - 12 
//respectively(for 10 bits each.); the final 12 - bits encode the patch version number.
//These handy macros should help with fetching some human readable digits from the encoded API integer.
#define VK_VER_MAJOR(X) ((((uint32_t)(X))>>22)&0x3FF)
#define VK_VER_MINOR(X) ((((uint32_t)(X))>>12)&0x3FF)
#define VK_VER_PATCH(X) (((uint32_t)(X)) & 0xFFF)


VulkanRendererInitializer::VulkanRendererInitializer(): m_bPrepared(false)
{

}




VulkanRendererInitializer::~VulkanRendererInitializer()
{
	// Clean up Vulkan resources
	if (m_DescriptorPool != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(m_SwapChain.device, m_DescriptorPool, nullptr);
	}

	if (m_SetupCmdBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, 1, &m_SetupCmdBuffer);
	}


	//destroy command buffers
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_DrawCmdBuffers.data());
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_PrePresentCmdBuffers.data());
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_PostPresentCmdBuffers.data());


	vkDestroyRenderPass(m_SwapChain.device, m_RenderPass, nullptr);
	for (uint32_t i = 0; i < m_FrameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(m_SwapChain.device, m_FrameBuffers[i], nullptr);
	}

	vkDestroyImageView(m_SwapChain.device,m_DepthStencil.view, nullptr);
	vkDestroyImage(m_SwapChain.device, m_DepthStencil.image, nullptr);
	vkFreeMemory(m_SwapChain.device, m_DepthStencil.mem, nullptr);

	vkDestroyPipelineCache(m_SwapChain.device, m_PipelineCache, nullptr);
	vkDestroyFence(m_SwapChain.device, m_Fence, nullptr);
	vkDestroyCommandPool(m_SwapChain.device, m_CmdPool, nullptr);

	vkDestroySemaphore(m_SwapChain.device, m_Semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_SwapChain.device, m_Semaphores.renderComplete, nullptr);
	vkDestroySemaphore(m_SwapChain.device, m_Semaphores.textOverlayComplete, nullptr);

	//Clean swapchain
	m_SwapChain.Cleanup();

	vkDestroyDevice(m_SwapChain.device, nullptr);
	vkDestroyInstance(m_SwapChain.instance, nullptr);

#if defined(__linux)
#if defined(__ANDROID__)
	// todo : android cleanup (if required)
#else
	xcb_destroy_window(connection, window);
	xcb_disconnect(connection);
#endif
#endif
}



void VulkanRendererInitializer::Render()
{
	
}

bool VulkanRendererInitializer::CreateRendererScreen(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM))
{
	//Do not initialize again if this instance was initialized already
	if (m_bPrepared) {
		return m_bPrepared;
	}

#if defined (_WIN32)
	CreateWindows(widht, height, MainWindowProc);

#elif defined (__ANDROID__)

#elif defined (__linux__)

#endif


	VkResult err;
#ifdef _DEBUG
	//enalbe validation during debug mode
	err = CreateVulkanInstance(true);
#else
	//disable validation during release mode
	err = CreateVulkanInstance(false);
#endif

	if(err)
	{
		fprintf(stdout, "Could not create Vulkan Instance %s\n", VkTools::VkResultToString(err));
		return false;
	}

	//Now that we have an instance we need a way to associate the instance with the hardware.
	//In Vulkan there is no notion of a singular GPU, instead you enumerate physical devices and choose.
	//This allows you to use multiple physical devices at the same time for rendering or compute.
	if (!GetPhysicalDevicesEnumerationAndProperties())
	{
		return false;
	}

	//Queues in Vulkan provide an interface to the execution engine of a device. 
	//Commands are recorded into command buffers ahead of execution time. 
	//These same buffers are then submitted to queues for execution. 
	//Each physical devices provides a family of queues to choose from. 
	//The choice of the queue depends on the task at hand.
	GetDeviceQueues();


	err = CreateDevice();
	if (err)
	{
		fprintf(stderr, "Could not create Vulkan Device %s\n", VkTools::VkResultToString(err));
		return false;
	}

	// Get the graphics queue
	vkGetDeviceQueue(m_SwapChain.device, m_graphicsQueueIndex, 0, &m_Queue);
	vkGetPhysicalDeviceMemoryProperties(m_SwapChain.physicalDevice, &m_DeviceMemoryProperties);
	vkGetPhysicalDeviceFeatures(m_SwapChain.physicalDevice, &m_DeviceFeatures);

	// Find a suitable depth format
	VkBool32 validDepthFormat = VkTools::GetSupportedDepthFormat(m_SwapChain.physicalDevice, m_SwapChain.depthFormat);
	assert(validDepthFormat);


	//Get all functions for swapchain
	m_SwapChain.Connect();


	//Create surface
#if defined(_WIN32)
	m_SwapChain.initSurface(m_hinstance, m_HwndWindows);
#elif defined(__ANDROID__)	
	m_SwapChain.initSurface(androidApp->window);
#elif defined(__linux__)
	m_SwapChain.initSurface(connection, window);
#endif


	// Create synchronization objects
	VkSemaphoreCreateInfo semaphoreCreateInfo = VkTools::Initializer::SemaphoreCreateInfo();

	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queu
	VK_CHECK_RESULT(vkCreateSemaphore(m_SwapChain.device, &semaphoreCreateInfo, nullptr, &m_Semaphores.presentComplete));

	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been sumbitted and executed
	VK_CHECK_RESULT(vkCreateSemaphore(m_SwapChain.device, &semaphoreCreateInfo, nullptr, &m_Semaphores.renderComplete));

	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands for the text overlay have been sumbitted and executed
	// Will be inserted after the render complete semaphore if the text overlay is enabled
	VK_CHECK_RESULT(vkCreateSemaphore(m_SwapChain.device, &semaphoreCreateInfo, nullptr, &m_Semaphores.textOverlayComplete));


	// Set up submit info structure
	// Semaphores will stay the same during application lifetime
	// Command buffer submission info is set by each example
	m_SubmitInfo = VkTools::Initializer::SubmitInfo();
	m_SubmitInfo.pWaitDstStageMask = &m_SubmitPipelineStages;
	m_SubmitInfo.waitSemaphoreCount = 1;
	m_SubmitInfo.pWaitSemaphores = &m_Semaphores.presentComplete;
	m_SubmitInfo.signalSemaphoreCount = 1;

	//Prepare all needed things for Vulkan
#ifdef _DEBUG
	PrepareVulkan(widht, height, true, true);
#else
	PrepareVulkan(widht, height);
#endif

	m_bPrepared = true;
	return m_bPrepared;
}

bool VulkanRendererInitializer::PrepareVulkan(uint32_t width, uint32_t height, bool benableValidation /* = false */, bool enableDebugMarkers /* = false*/)
{
	if (benableValidation)
	{
		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an appplication the error and warning bits should suffice
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT; // VK_DEBUG_REPORT_WARNING_BIT_EXT (enable to also display warnings)
																				// Additional flags include performance info, loader and layer debug messages, etc.
		vkDebug::setupDebugging(m_SwapChain.instance, debugReportFlags, VK_NULL_HANDLE);
	}
	if (enableDebugMarkers)
	{
		vkDebug::DebugMarker::setup(m_SwapChain.device);
	}

	CreateCommandPool();
	CreateSetupCommandBuffer();
	SetupSwapChain(width, height);
	CreateCommandBuffers();
	BuildPresentCommandBuffers();
	SetupDepthStencil(width, height);
	SetupRenderPass();
	CreatePipelineCache();
	SetupFrameBuffer(width, height);
	FlushSetupCommandBuffer();

	// Recreate setup command buffer for derived class
	CreateSetupCommandBuffer();

	//Command buffer execution fence
	VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo();
	VK_CHECK_RESULT(vkCreateFence(m_SwapChain.device, &fenceCreateInfo, nullptr, &m_Fence));

	return true;
}


bool VulkanRendererInitializer::CreateWindows(uint32_t width, uint32_t height, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM))
{
	bool fullscreen = false;
	WNDCLASSEX wndClass;



	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = MainWindowProc;
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = m_hinstance;
	wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = reinterpret_cast<LPCTSTR>(className.c_str());
	wndClass.hIconSm = LoadIcon(NULL, IDI_WINLOGO);

	if (!RegisterClassEx(&wndClass))
	{
		fprintf(stdout, "Could not create window class \n");
		return false;
	}

	int screenWidth = GetSystemMetrics(SM_CXSCREEN);
	int screenHeight = GetSystemMetrics(SM_CYSCREEN);

	if (fullscreen)
	{
		DEVMODE dmScreenSettings;
		memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
		dmScreenSettings.dmSize = sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth = screenWidth;
		dmScreenSettings.dmPelsHeight = screenHeight;
		dmScreenSettings.dmBitsPerPel = 32;
		dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		if ((width != screenWidth) && (height != screenHeight))
		{
			if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
			{
				if (MessageBox(NULL, TEXT("Fullscreen Mode not supported!\n Switch to window mode?"), TEXT("Error"), MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
				{
					fullscreen = FALSE;
				}
				else
				{
					return FALSE;
				}
			}
		}

	}

	DWORD dwExStyle;
	DWORD dwStyle;

	if (fullscreen)
	{
		dwExStyle = WS_EX_APPWINDOW;
		dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	}
	else
	{
		dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		dwStyle = WINDOW_STYLE | WS_SYSMENU;
	}

	RECT windowRect;
	if (fullscreen)
	{
		windowRect.left = (long)0;
		windowRect.right = (long)screenWidth;
		windowRect.top = (long)0;
		windowRect.bottom = (long)screenHeight;
	}
	else
	{
		windowRect.left = (long)screenWidth / 2 - width / 2;
		windowRect.right = (long)width;
		windowRect.top = (long)screenHeight / 2 - height / 2;
		windowRect.bottom = (long)height;
	}
	AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);

	m_HwndWindows = CreateWindowEx(
		dwExStyle,
		reinterpret_cast<LPCTSTR>(className.c_str()),
		reinterpret_cast<LPCTSTR>(titleName.c_str()),
		dwStyle,
		windowRect.left,
		windowRect.top,
		windowRect.right,
		windowRect.bottom,
		NULL,
		NULL,
		m_hinstance,
		NULL);

	if (!m_HwndWindows)
	{
		printf("Could not create window!\n");
		fflush(stdout);
		return 0;
		exit(1);
	}

	ShowWindow(m_HwndWindows, SW_SHOW);
	UpdateWindow(m_HwndWindows);
	SetForegroundWindow(m_HwndWindows);
	SetFocus(m_HwndWindows);
}

VkResult VulkanRendererInitializer::CreateVulkanInstance(bool bEnableValidation)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = className.c_str();
	appInfo.pEngineName = className.c_str();
	appInfo.apiVersion = VK_API_VERSION_1_0;
	appInfo.pNext = NULL;
	//appInfo.engineVersion = 


	std::vector<const char*> enabledExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };

	// Enable surface extensions depending on os
#if defined(_WIN32)
	enabledExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);
#elif defined(__ANDROID__)
	enabledExtensions.push_back(VK_KHR_ANDROID_SURFACE_EXTENSION_NAME);
#elif defined(__linux__)
	enabledExtensions.push_back(VK_KHR_XCB_SURFACE_EXTENSION_NAME);
#endif


	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = NULL;
	instanceCreateInfo.flags = 0;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (enabledExtensions.size() > 0)
	{
		if (bEnableValidation)
		{
			enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
		}
		instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}
	if (bEnableValidation)
	{
		instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
		instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
	}
	return vkCreateInstance(&instanceCreateInfo, nullptr, &m_SwapChain.instance);
}


bool VulkanRendererInitializer::GetPhysicalDevicesEnumerationAndProperties()
{
	// Query how many devices are present in the system
	uint32_t deviceCount = 0;
	VkResult result = vkEnumeratePhysicalDevices(m_SwapChain.instance, &deviceCount, NULL);
	if (result != VK_SUCCESS) 
	{
		fprintf(stderr, "Failed to query the number of physical devices present: %d\n", result);
		return false;
	}

	// There has to be at least one device present
	if (deviceCount == 0) 
	{
		fprintf(stderr, "Couldn't detect any device present with Vulkan support: %d\n", result);
		return false;
	}

	// Get the physical devices
	m_PhysicalDevices.resize(deviceCount);
	result = vkEnumeratePhysicalDevices(m_SwapChain.instance, &deviceCount, &m_PhysicalDevices[0]);
	if (result != VK_SUCCESS) 
	{
		fprintf(stderr, "Faied to enumerate physical devices present: %d\n", result);
		return false;
	}


	// Enumerate all physical devices
	VkPhysicalDeviceProperties deviceProperties;
	VkPhysicalDeviceFeatures deviceFeatures;

	for (uint32_t i = 0; i < deviceCount; i++)
	{
		memset(&deviceProperties, 0, sizeof deviceProperties);
		vkGetPhysicalDeviceProperties(m_PhysicalDevices[i], &deviceProperties);

		printf("Driver Version: %d\n", deviceProperties.driverVersion);
		printf("Device Name:    %s\n", deviceProperties.deviceName);
		printf("Device Type:    %d\n", deviceProperties.deviceType);
		printf("API Version:    %d.%d.%d\n",
			// See note regarding this:
			VK_VER_MAJOR(deviceProperties.apiVersion),
			VK_VER_MINOR(deviceProperties.apiVersion),
			VK_VER_PATCH(deviceProperties.apiVersion));
	}
	return true;
}


void VulkanRendererInitializer::GetDeviceQueues()
{

	// Note :
	// This example will always use the first physical device reported,
	// change the vector index if you have multiple Vulkan devices installed
	// and want to use another one
	m_SwapChain.physicalDevice = m_PhysicalDevices[0];


	// Find a queue that supports graphics operations
	uint32_t queueFamilyCount = 0;

	vkGetPhysicalDeviceQueueFamilyProperties(m_SwapChain.physicalDevice, &queueFamilyCount, nullptr);
	m_QueueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_SwapChain.physicalDevice, &queueFamilyCount, m_QueueFamilyProperties.data());


	// Get the graphics queue
	m_graphicsQueueIndex = 0;
	for (uint32_t j = 0; j < queueFamilyCount; j++)
	{
		printf("Count of Queues: %d\n", m_QueueFamilyProperties[j].queueCount);
		printf("Supported operationg on this queue:\n");
		if (m_QueueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			printf("\t\t Graphics\n");
			m_graphicsQueueIndex = j;
		}
		if (m_QueueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
		{
			printf("\t\t Compute\n");
		}
		if (m_QueueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
		{
			printf("\t\t Transfer\n");
		}
		if (m_QueueFamilyProperties[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
		{
			printf("\t\t Sparse Binding\n");
		}
	}
}


VkResult VulkanRendererInitializer::CreateDevice()
{
	// Here's where we initialize our queues
	std::array<float, 1> queuePriorities = { 0.0f };
	VkDeviceQueueCreateInfo deviceQueueInfo;
	deviceQueueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	deviceQueueInfo.queueFamilyIndex = m_graphicsQueueIndex;
	deviceQueueInfo.queueCount = 1;
	deviceQueueInfo.pQueuePriorities = queuePriorities.data();
	deviceQueueInfo.pNext = NULL;
	deviceQueueInfo.flags = 0;



	std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	VkPhysicalDeviceFeatures enabledFeatures = {};
	enabledFeatures.tessellationShader = true;
	enabledFeatures.shaderTessellationAndGeometryPointSize = true;
	enabledFeatures.geometryShader = true;
	enabledFeatures.shaderClipDistance = true;
	enabledFeatures.shaderCullDistance = true;

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.pNext = NULL;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &deviceQueueInfo;
	deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

	// enable the debug marker extension if it is present (likely meaning a debugging tool is present)
	if (VkTools::CheckDeviceExtensionPresent(m_SwapChain.physicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		//enableDebugMarkers = true;
	}

	if (enabledExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
		deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
	}


	return vkCreateDevice(m_SwapChain.physicalDevice, &deviceCreateInfo, nullptr, &m_SwapChain.device);
}


bool VulkanRendererInitializer::CreateSurface()
{
	m_SwapChain.fpGetPhysicalDeviceSurfaceFormatsKHR = nullptr;
	
	// somewhere in initialization code
	*(void **)&m_SwapChain.fpGetPhysicalDeviceSurfaceFormatsKHR = vkGetInstanceProcAddr(m_SwapChain.instance, "vkGetPhysicalDeviceSurfaceFormatsKHR");
	if (!m_SwapChain.fpGetPhysicalDeviceSurfaceFormatsKHR) {
		return false;
	}

#if defined(_WIN32)
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance =  m_hinstance;       // provided by the platform code
	surfaceCreateInfo.hwnd = m_HwndWindows;           // provided by the platform code
	VkResult result = vkCreateWin32SurfaceKHR(m_SwapChain.instance, &surfaceCreateInfo, NULL, &m_SwapChain.surface);
#elif defined(__ANDROID__)
	VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.window = window;                       // provided by the platform code
	VkResult result = vkCreateAndroidSurfaceKHR(m_SwapChain.instance, &surfaceCreateInfo, NULL, &m_SwapChain.surface);
#elif defined(__linux__)
	VkXcbSurfaceCreateInfoKHR surfaceCreateInfo;
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.connection = connection;               // provided by the platform code
	surfaceCreateInfo.window = window;                       // provided by the platform code
	VkResult result = vkCreateXcbSurfaceKHR(m_SwapChain.instance, &surfaceCreateInfo, NULL, &m_SwapChain.surface);
#endif

	if (result != VK_SUCCESS) {
		fprintf(stderr, "Failed to create Vulkan surface: %d\n", result);
		return false;
	}


	uint32_t formatCount = 0;
	m_SwapChain.fpGetPhysicalDeviceSurfaceFormatsKHR(m_SwapChain.physicalDevice, m_SwapChain.surface, &formatCount, NULL);
	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	m_SwapChain.fpGetPhysicalDeviceSurfaceFormatsKHR(m_SwapChain.physicalDevice, m_SwapChain.surface, &formatCount, &surfaceFormats[0]);

	// If the format list includes just one entry of VK_FORMAT_UNDEFINED,
	// the surface has no preferred format. Otherwise, at least one
	// supported format will be returned
	if (formatCount == 1 && surfaceFormats[0].format == VK_FORMAT_UNDEFINED)
	{
		m_SwapChain.colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
	}
	else 
	{
		assert(formatCount >= 1);
		m_SwapChain.colorFormat = surfaceFormats[0].format;
	}
	m_SwapChain.colorSpace = surfaceFormats[0].colorSpace;

	return true;
}



void SetImageLayout(VkCommandBuffer commandBuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout)
{
	VkImageMemoryBarrier imageMemoryBarrier;
	imageMemoryBarrier.oldLayout = oldImageLayout;
	imageMemoryBarrier.newLayout = newImageLayout;
	imageMemoryBarrier.image = image;
	imageMemoryBarrier.subresourceRange.aspectMask = aspectMask;
	imageMemoryBarrier.subresourceRange.baseMipLevel = 0;
	imageMemoryBarrier.subresourceRange.levelCount = 1;
	imageMemoryBarrier.subresourceRange.layerCount = 1;

	// Undefined layout:
	//   Note: Only allowed as initial layout!
	//   Note: Make sure any writes to the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_UNDEFINED)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;

	// Old layout is color attachment:
	//   Note: Make sure any writes to the color buffer have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	// Old layout is transfer source:
	//   Note: Make sure any reads from the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

	// Old layout is shader read (sampler, input attachment):
	//   Note: Make sure any shader reads from the image have been finished
	if (oldImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_READ_BIT;

	// New layout is transfer destination (copy, blit):
	//   Note: Make sure any copyies to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

	// New layout is transfer source (copy, blit):
	//   Note: Make sure any reads from and writes to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = imageMemoryBarrier.srcAccessMask | VK_ACCESS_TRANSFER_READ_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is color attachment:
	//   Note: Make sure any writes to the color buffer hav been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL) {
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
	}

	// New layout is depth attachment:
	//   Note: Make sure any writes to depth/stencil buffer have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
		imageMemoryBarrier.dstAccessMask = imageMemoryBarrier.dstAccessMask | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// New layout is shader read (sampler, input attachment):
	//   Note: Make sure any writes to the image have been finished
	if (newImageLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		imageMemoryBarrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT | VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}

	// Put barrier inside the setup command buffer
	vkCmdPipelineBarrier(commandBuffer,
		// Put the barriers for source and destination on
		// top of the command buffer
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		0,
		0, NULL,
		0, NULL,
		1, &imageMemoryBarrier);
}

void VulkanRendererInitializer::DestroyRendererScreen()
{
	//g_RendGL.Log("Shutting down Vulkan subsystem");
}

void VulkanRendererInitializer::RendererSwapBuffers()
{

}

/////////////////////////////////////////////////////////////////////////////////////
//								PREPARE STAGE
/////////////////////////////////////////////////////////////////////////////////////

void VulkanRendererInitializer::CreateCommandPool()
{
	//Create command pool
	VkCommandPoolCreateInfo cmdPoolInfo = {};
	cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	cmdPoolInfo.queueFamilyIndex = m_SwapChain.queueNodeIndex;
	cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VK_CHECK_RESULT(vkCreateCommandPool(m_SwapChain.device, &cmdPoolInfo, nullptr, &m_CmdPool));
}

void VulkanRendererInitializer::CreateSetupCommandBuffer(uint32_t& width, uint32_t& height)
{
	//Settup command buffers
	if (m_SetupCmdBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, 1, &m_SetupCmdBuffer);
		m_SetupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
	}
	VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(m_CmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_SwapChain.device, &cmdBufAllocateInfo, &m_SetupCmdBuffer));

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VK_CHECK_RESULT(vkBeginCommandBuffer(m_SetupCmdBuffer, &cmdBufInfo));

	//Setup swapchain
	m_SwapChain.Create(m_SetupCmdBuffer, width, height, false);
}

void VulkanRendererInitializer::SetupSwapChain(uint32_t& width, uint32_t& height)
{
	m_SwapChain.Create(m_SetupCmdBuffer, width, height, false);
}

void VulkanRendererInitializer::CreateCommandBuffers()
{
	// Create one command buffer per frame buffer
	// in the swap chain
	// Command buffers store a reference to the
	// frame buffer inside their render pass info
	// so for static usage withouth having to rebuild
	// them each frame, we use one per frame buffer
	m_DrawCmdBuffers.resize(m_SwapChain.imageCount);
	m_PrePresentCmdBuffers.resize(m_SwapChain.imageCount);
	m_PostPresentCmdBuffers.resize(m_SwapChain.imageCount);

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(m_CmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, static_cast<uint32_t>(m_DrawCmdBuffers.size()));

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_SwapChain.device, &cmdBufAllocateInfo, m_DrawCmdBuffers.data()));

	// Command buffers for submitting present barriers
	// One pre and post present buffer per swap chain image
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_SwapChain.device, &cmdBufAllocateInfo, m_PrePresentCmdBuffers.data()));
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_SwapChain.device, &cmdBufAllocateInfo, m_PostPresentCmdBuffers.data()));
}

void VulkanRendererInitializer::BuildPresentCommandBuffers()
{
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	for (uint32_t i = 0; i < m_SwapChain.imageCount; i++)
	{
		// Command buffer for post present barrier

		// Insert a post present image barrier to transform the image back to a
		// color attachment that our render pass can write to
		// We always use undefined image layout as the source as it doesn't actually matter
		// what is done with the previous image contents

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_PostPresentCmdBuffers[i], &cmdBufInfo));

		VkImageMemoryBarrier postPresentBarrier = VkTools::Initializer::ImageMemoryBarrier();
		postPresentBarrier.srcAccessMask = 0;
		postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		postPresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		postPresentBarrier.image = m_SwapChain.buffers[i].image;

		vkCmdPipelineBarrier(
			m_PostPresentCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			0,
			0, nullptr,
			0, nullptr,
			1, &postPresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_PostPresentCmdBuffers[i]));

		// Command buffers for pre present barrier

		// Submit a pre present image barrier to the queue
		// Transforms the (framebuffer) image layout from color attachment to present(khr) for presenting to the swap chain

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_PrePresentCmdBuffers[i], &cmdBufInfo));

		VkImageMemoryBarrier prePresentBarrier = VkTools::Initializer::ImageMemoryBarrier();
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		prePresentBarrier.image = m_SwapChain.buffers[i].image;

		vkCmdPipelineBarrier(
			m_PrePresentCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_FLAGS_NONE,
			0, nullptr, // No memory barriers,
			0, nullptr, // No buffer barriers,
			1, &prePresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_PrePresentCmdBuffers[i]));
	}
}

void VulkanRendererInitializer::SetupDepthStencil(uint32_t& width, uint32_t& height)
{
	VkImageCreateInfo image = {};
	image.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image.pNext = NULL;
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = m_SwapChain.depthFormat;
	image.extent = { width, height, 1 };
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	image.flags = 0;

	VkMemoryAllocateInfo mem_alloc = {};
	mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	mem_alloc.pNext = NULL;
	mem_alloc.allocationSize = 0;
	mem_alloc.memoryTypeIndex = 0;

	VkImageViewCreateInfo depthStencilView = {};
	depthStencilView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	depthStencilView.pNext = NULL;
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = m_SwapChain.depthFormat;
	depthStencilView.flags = 0;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;

	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(m_SwapChain.device, &image, nullptr, &m_DepthStencil.image));
	vkGetImageMemoryRequirements(m_SwapChain.device, m_DepthStencil.image, &memReqs);
	mem_alloc.allocationSize = memReqs.size;
	mem_alloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_DeviceMemoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(m_SwapChain.device, &mem_alloc, nullptr, &m_DepthStencil.mem));

	VK_CHECK_RESULT(vkBindImageMemory(m_SwapChain.device, m_DepthStencil.image, m_DepthStencil.mem, 0));
	VkTools::SetImageLayout(
		m_SetupCmdBuffer,
		m_DepthStencil.image,
		VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	depthStencilView.image = m_DepthStencil.image;
	VK_CHECK_RESULT(vkCreateImageView(m_SwapChain.device, &depthStencilView, nullptr, &m_DepthStencil.view));
}


void VulkanRendererInitializer::SetupRenderPass()
{
	VkAttachmentDescription attachments[2] = {};

	// Color attachment
	attachments[0].format = m_SwapChain.colorFormat;
	attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	// Depth attachment
	attachments[1].format = m_SwapChain.depthFormat;
	attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachments[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.flags = 0;
	subpass.inputAttachmentCount = 0;
	subpass.pInputAttachments = NULL;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pResolveAttachments = NULL;
	subpass.pDepthStencilAttachment = &depthReference;
	subpass.preserveAttachmentCount = 0;
	subpass.pPreserveAttachments = NULL;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pNext = NULL;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = attachments;
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 0;
	renderPassInfo.pDependencies = NULL;

	VK_CHECK_RESULT(vkCreateRenderPass(m_SwapChain.device, &renderPassInfo, nullptr, &m_RenderPass));
}



void VulkanRendererInitializer::CreatePipelineCache()
{
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	VK_CHECK_RESULT(vkCreatePipelineCache(m_SwapChain.device, &pipelineCacheCreateInfo, nullptr, &m_PipelineCache));
}

void VulkanRendererInitializer::SetupFrameBuffer(uint32_t& width, uint32_t& height)
{
	VkImageView attachments[2];

	// Depth/Stencil attachment is the same for all frame buffers
	attachments[1] = m_DepthStencil.view;

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = NULL;
	frameBufferCreateInfo.renderPass = m_RenderPass;
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	// Create frame buffers for every swap chain image
	m_FrameBuffers.resize(m_SwapChain.imageCount);
	for (uint32_t i = 0; i < m_FrameBuffers.size(); i++)
	{
		attachments[0] = m_SwapChain.buffers[i].view;
		VK_CHECK_RESULT(vkCreateFramebuffer(m_SwapChain.device, &frameBufferCreateInfo, nullptr, &m_FrameBuffers[i]));
	}
}

void VulkanRendererInitializer::FlushSetupCommandBuffer()
{
	if (m_SetupCmdBuffer == VK_NULL_HANDLE)
		return;

	VK_CHECK_RESULT(vkEndCommandBuffer(m_SetupCmdBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &m_SetupCmdBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(m_Queue));

	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, 1, &m_SetupCmdBuffer);
	m_SetupCmdBuffer = VK_NULL_HANDLE;
}

// Recreate setup command buffer for derived class
void VulkanRendererInitializer::CreateSetupCommandBuffer()
{
	if (m_SetupCmdBuffer != VK_NULL_HANDLE)
	{
		vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, 1, &m_SetupCmdBuffer);
		m_SetupCmdBuffer = VK_NULL_HANDLE; // todo : check if still necessary
	}

	VkCommandBufferAllocateInfo cmdBufAllocateInfo =VkTools::Initializer::CommandBufferAllocateInfo(m_CmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY,1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_SwapChain.device, &cmdBufAllocateInfo, &m_SetupCmdBuffer));

	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	VK_CHECK_RESULT(vkBeginCommandBuffer(m_SetupCmdBuffer, &cmdBufInfo));
}


void VulkanRendererInitializer::WindowResize(uint32_t width, uint32_t height)
{
	if (!m_bPrepared)
	{
		return;
	}
	m_bPrepared = false;

	// Recreate swap chain
	CreateSetupCommandBuffer();
	SetupSwapChain(width, height);

	// Recreate the frame buffers

	vkDestroyImageView(m_SwapChain.device, m_DepthStencil.view, nullptr);
	vkDestroyImage(m_SwapChain.device, m_DepthStencil.image, nullptr);
	vkFreeMemory(m_SwapChain.device, m_DepthStencil.mem, nullptr);
	SetupDepthStencil(width, height);

	for (uint32_t i = 0; i < m_FrameBuffers.size(); i++)
	{
		vkDestroyFramebuffer(m_SwapChain.device, m_FrameBuffers[i], nullptr);
	}
	SetupFrameBuffer(width, height);

	FlushSetupCommandBuffer();

	// Command buffers need to be recreated as they may store
	// references to the recreated frame buffer
	DestroyCommandBuffers();
	CreateCommandBuffers();

	BuildPresentCommandBuffers();
}


void VulkanRendererInitializer::DestroyCommandBuffers()
{
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_DrawCmdBuffers.data());
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_PrePresentCmdBuffers.data());
	vkFreeCommandBuffers(m_SwapChain.device, m_CmdPool, static_cast<uint32_t>(m_DrawCmdBuffers.size()), m_PostPresentCmdBuffers.data());
}