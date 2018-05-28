#pragma once
#include <glm\vec2.hpp>
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DOD.h"

namespace Renderer
{
	namespace Vulkan
	{
		#define SECONDARY_COMMAND_BUFFER_COUNT 128u

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
			static glm::uvec2                    backBufferDimensions;

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

			static void Shutdown();

			static void InitVulkanInstance(bool enableValidation, const std::string& application_name);

			static void InitVulkanDebug(bool enableValidation);
			static void DestroyVulkanDebug(bool enableValidation);

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

			static void InitOrUpdateVulkanSwapChain(bool vsync);
			static void InitVulkanPipelineCache();
			static void InitPlatformDependentFormats();
			static void InitVulkanSynchronization();

			static void ResizeSwapchain();

			static void StartFrame();
			static void EndFrame();

			static bool WaitForFrame(const uint32_t index);

			static void BeginPrimaryCommandBuffer();
			static void EndPrimaryCommandBuffer();

			static void BeginSecondaryComandBuffer(uint32_t p_CmdBufferIdx, VkRenderPass p_VkRenderPass, VkFramebuffer p_VkFramebuffer);
			static void EndSecondaryComandBuffer(uint32_t p_CmdBufferId);

			static void BeginRenderPass(const DOD::Ref& renderPass, const DOD::Ref& frameBufferRef, 
				VkSubpassContents p_SubpassContents = VK_SUBPASS_CONTENTS_INLINE, uint32_t clearValueCount = 0u, VkClearValue* p_ClearValues = nullptr);

			static void EndRenderPass()
			{
				vkCmdEndRenderPass(GetPrimaryCommandBuffer());
			}

			static void InsertPostPresentBarrier();
			static void InsertPrePresentBarrier();

			static VkCommandBuffer GetPrimaryCommandBuffer()
			{
				return vkPrimalCommandBuffers[backBufferIndex];
			}

			static VkCommandBuffer& GetSecondaryCommandBuffer(uint32_t commandBufferIndex)
			{
				return vkSecondaryCommandBuffers[backBufferIndex * SECONDARY_COMMAND_BUFFER_COUNT + commandBufferIndex];
			}
		};
	}
}