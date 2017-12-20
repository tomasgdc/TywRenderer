#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>

namespace Renderer
{
	namespace Vulkan
	{
		struct RenderSystem
		{
			static std::vector<VkCommandBuffer> vkPrimalCommandBuffers;
			static VkCommandPool                vkPrimalCommandPool;
			static std::vector<VkCommandBuffer> vkSecondaryCommandBuffers;
			static std::vector<VkCommandPool>   vkSecondaryCommandPools;
			static std::vector<VkImage>			vkSwapchainImages;
			static std::vector<VkFence>         vkDrawFences;

			static VkDevice                     vkDevice;
			static VkInstance					vkInstance;

			static VkPhysicalDevice             vkPhysicalDevice;
			static VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties;

			static VkQueue                       vkQueue;
			static uint32_t                      vkGraphicsQueueFamilyIndex;

			static VkSurfaceKHR                  vkSurface;
			static VkSwapchainKHR                vkSwapchain;
			static std::vector<VkImageView>      vkSwapchainImageViews;
			static VkFormat                      vkDepthFormatToUse;
			static VkFormat                      vkColorFormatToUse;
			static VkSemaphore                   vkImageAcquireSemaphore;

			static uint32_t                      backBufferIndex;
			static uint32_t                      activeBackbufferMask;

			static VkPipelineCache               vkPipelineCache;

			static void Init(
				bool enableValidation,
				bool enableVsync,
				const std::string& application_name,
#ifdef _WIN32
				HINSTANCE handle,
				HWND window
#else
#ifdef __ANDROID__
				ANativeWindow* window
#else
				xcb_connection_t* connection, xcb_window_t window
#endif
#endif
			);

			static void InitVulkanInstance(bool enableValidation, const std::string& application_name);
			static void InitVulkanDevice(bool enableValidation);
			static void InitCommandPool();

			static void InitCommandBuffers();
			static void DestroyCommandBuffers();

			static void InitVulkanSurface(
#ifdef _WIN32
				HINSTANCE handle,
				HWND window
#else
#ifdef __ANDROID__
				ANativeWindow* window
#else
				xcb_connection_t* connection, xcb_window_t window
#endif
#endif
			);

			static void InitVulkanSwapChain(bool vsync);
			static void InitVulkanPipelineCache();
			static void InitPlatformDependentFormats();
			static void InitVulkanSynchronization();

			static void StartFrame();
			static void EndFrame();

			static bool WaitForFrame(const uint32_t index);

			static void BeginPrimaryCommandBuffer();
			static void EndPrimaryCommandBuffer();

			static void BeginSecondaryComandBuffer(uint32_t p_CmdBufferIdx, VkRenderPass p_VkRenderPass, VkFramebuffer p_VkFramebuffer);

			static void InsertPostPresentBarrier();
			static void InsertPrePresentBarrier();

			static VkCommandBuffer GetPrimaryCommandBuffer()
			{
				return vkPrimalCommandBuffers[backBufferIndex];
			}
		};
	}
}