/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include "Vulkan/VulkanSwapChain.h"
#include "Vulkan\VulkanTools.h"
#include "IRendererInitializer.h"


#define	WINDOW_STYLE	(WS_OVERLAPPED|WS_BORDER|WS_CAPTION|WS_VISIBLE | WS_THICKFRAME)



class VulkanRendererInitializer final: public IRendererInitializer
{

public:
	VulkanRendererInitializer();
	~VulkanRendererInitializer();

	bool CreateRendererScreen(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM));
	void DestroyRendererScreen();


	/*
	if interval = 0 then vsync is disabled
	if interval = 1 then vsync is enabled
	if intervak = -1 then adaptive vsync is enabled

	Adaptive vsync enables v-blank synchronisation when the frame rate is higher than the sync rate,
	but disables synchronisation when the frame rate drops below the sync rate.
	Disabling the synchronisation on low frame rates prevents the common problem where the frame rate
	syncs to a integer fraction of the screen's refresh rate in a complex scene.
	*/
	void RendererSwapBuffers();

	HWND GetWind32Handle() { return m_HwndWindows; }
public:
	VkResult CreateVulkanInstance(bool bEnableValidation);

	//Gets list of available GPUs and they're description
	bool GetPhysicalDevicesEnumerationAndProperties();

	//Gets a list of available GPU command queues
	void GetDeviceQueues();


	VkResult CreateDevice();


	bool CreateSurface();


	//Create window screen depended on os
	bool CreateWindows(uint32_t width, uint32_t height, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM));


	bool PrepareVulkan(uint32_t width, uint32_t height, bool benableValidation = false, bool enableDebugMarkers = false);

public:

	void CreateCommandPool();

	void CreateSetupCommandBuffer(uint32_t& width, uint32_t& height);

	void SetupSwapChain(uint32_t& width, uint32_t& heigh);
	
	void CreateCommandBuffers();
	
	void BuildPresentCommandBuffers();
	
	void SetupDepthStencil(uint32_t& width, uint32_t& height);
	
	void SetupRenderPass();
	
	void CreatePipelineCache();
	
	void SetupFrameBuffer(uint32_t& width, uint32_t& height);
	
	void FlushSetupCommandBuffer();
	
	// Recreate setup command buffer for derived class
	void CreateSetupCommandBuffer();


	void Render();


	void WindowResize(uint32_t width, uint32_t height);


	void DestroyCommandBuffers();

public:
	// Command buffers used for rendering
	static std::vector<VkCommandBuffer>			m_DrawCmdBuffers;

	// Command buffers for submitting a pre present image barrier
	std::vector<VkCommandBuffer>			m_PrePresentCmdBuffers = { VK_NULL_HANDLE };

	// Command buffer for submitting a post present image barrier
	std::vector<VkCommandBuffer>			m_PostPresentCmdBuffers = { VK_NULL_HANDLE };

	//Properties
	std::vector<VkPhysicalDevice>			m_PhysicalDevices;
	std::vector<VkQueueFamilyProperties>	m_QueueFamilyProperties;

	static VulkanSwapChain							m_SwapChain;
	VkSemaphores							m_Semaphores;
	VkTools::VkDepthStencil					m_DepthStencil;

	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo							m_SubmitInfo;

	// Pipeline stage flags for the submit info structure
	VkPipelineStageFlags					m_SubmitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

	// Command buffer pool
	static VkCommandPool							m_CmdPool;

	// Command buffer used for setup
	VkCommandBuffer							m_SetupCmdBuffer = VK_NULL_HANDLE;

	// Stores all available memory (type) properties for the physical device
	VkPhysicalDeviceMemoryProperties		 m_DeviceMemoryProperties;

	VkPhysicalDeviceFeatures				m_DeviceFeatures;

	// Global render pass for frame buffer writes
	VkRenderPass							m_RenderPass;

	// Pipeline cache object
	static VkPipelineCache							m_PipelineCache;

	// List of available frame buffers (same as number of swap chain images)
	static std::vector<VkFramebuffer>				m_FrameBuffers;

	// Handle to the device graphics queue that command buffers are submitted to
	static VkQueue									m_Queue;

	// Descriptor set pool
	VkDescriptorPool						m_DescriptorPool = VK_NULL_HANDLE;


	VkFence									m_Fence;
#if defined(_WIN32)
	HINSTANCE								m_hinstance;
	HWND									m_HwndWindows;
#elif defined(__ANDROID__)

#elif defined(linux)

#endif

	//Grapic index
	uint32_t								m_graphicsQueueIndex;
	
	// Active frame buffer index
	uint32_t								m_currentBuffer = 0;


	bool									m_bPrepared;


};