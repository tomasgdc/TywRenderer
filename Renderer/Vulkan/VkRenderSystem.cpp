#include "VkRenderSystem.h"
#include "VulkanTools.h"
#include "VulkanDebug.h"

#include "VkPipelineManager.h"
#include "VkRenderPassManager.h"
#include "VkPipelineLayoutManager.h"
#include "VkGpuProgram.h"
#include "VkBufferLayoutManager.h"
#include "VkBufferObjectManager.h"
#include "DrawCallManager.h"
#include "VkDrawCallDispatcher.h"
#include "VkGPUMemoryManager.h"
#include "VkImageManager.h"
#include "VkFrameBufferManager.h"



//In Vulkan the API version is encoded as a 32 - bit integer with the major and minor version being encoded into bits 31 - 22 and 21 - 12 
//respectively(for 10 bits each.); the final 12 - bits encode the patch version number.
//These handy macros should help with fetching some human readable digits from the encoded API integer.
#define VK_VER_MAJOR(X) ((((uint32_t)(X))>>22)&0x3FF)
#define VK_VER_MINOR(X) ((((uint32_t)(X))>>12)&0x3FF)
#define VK_VER_PATCH(X) (((uint32_t)(X)) & 0xFFF)

namespace Renderer
{
	namespace Vulkan
	{
		std::vector<VkCommandBuffer> RenderSystem::vkPrimalCommandBuffers;
		VkCommandPool                RenderSystem::vkPrimalCommandPool;
		std::vector<VkCommandBuffer> RenderSystem::vkSecondaryCommandBuffers;
		std::vector<VkCommandPool>   RenderSystem::vkSecondaryCommandPools;
		std::vector<VkImage>		 RenderSystem::vkSwapchainImages;
		std::vector<VkFence>         RenderSystem::vkDrawFences;

		VkDevice                     RenderSystem::vkDevice = nullptr;
		VkInstance					 RenderSystem::vkInstance = nullptr;

		VkPhysicalDevice			 RenderSystem::vkPhysicalDevice = nullptr;
		VkPhysicalDeviceMemoryProperties RenderSystem::vkPhysicalDeviceMemoryProperties;

		uint32_t                     RenderSystem::vkGraphicsQueueFamilyIndex = 0;
		VkQueue                      RenderSystem::vkQueue;


		VkSurfaceKHR                 RenderSystem::vkSurface = nullptr;
		VkSwapchainKHR               RenderSystem::vkSwapchain = nullptr;
		std::vector<VkImageView>     RenderSystem::vkSwapchainImageViews;
		VkFormat                     RenderSystem::vkDepthFormatToUse;
		VkFormat                     RenderSystem::vkColorFormatToUse = VK_FORMAT_B8G8R8A8_UNORM;
		VkSemaphore                  RenderSystem::vkImageAcquireSemaphore;

		uint32_t                     RenderSystem::backBufferIndex = 0u;
		uint32_t                     RenderSystem::activeBackbufferMask = 0u;
		glm::uvec2                   RenderSystem::backBufferDimensions = glm::uvec2(0, 0);

		VkPipelineCache              RenderSystem::vkPipelineCache = nullptr;

		void RenderSystem::InitVulkanInstance(bool enableValidation, const std::string& application_name)
		{
			/* Create device */
			VkApplicationInfo appInfo = {};
			appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
			appInfo.pApplicationName = application_name.c_str();
			appInfo.pEngineName = application_name.c_str();
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
				if (enableValidation)
				{
					enabledExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
				}
				instanceCreateInfo.enabledExtensionCount = (uint32_t)enabledExtensions.size();
				instanceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
			}
			if (enableValidation)
			{
				instanceCreateInfo.enabledLayerCount = vkDebug::validationLayerCount;
				instanceCreateInfo.ppEnabledLayerNames = vkDebug::validationLayerNames;
			}

			VK_CHECK_RESULT(vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance));
		}

		void RenderSystem::Shutdown()
		{
			//wait on the host for the completion of outstanding queue operations for all queues on a given logical device
			vkDeviceWaitIdle(vkDevice);

			Renderer::Vulkan::RenderSystem::DestroyCommandBuffers();

			//Release resources
			Renderer::Resource::DrawCallManager::DestroyResources(Renderer::Resource::DrawCallManager::activeRefs);
			Renderer::Resource::RenderPassManager::DestroyResources(Renderer::Resource::RenderPassManager::activeRefs);
			Renderer::Resource::GpuProgramManager::DestroyResources(Renderer::Resource::GpuProgramManager::activeRefs);
			Renderer::Resource::PipelineLayoutManager::DestroyResources(Renderer::Resource::PipelineLayoutManager::activeRefs);
			Renderer::Resource::PipelineManager::DestroyResources(Renderer::Resource::PipelineManager::activeRefs);
			Renderer::Resource::BufferObjectManager::DestroyResources(Renderer::Resource::BufferObjectManager::activeRefs);
			Renderer::Vulkan::GpuMemoryManager::Destroy();

			//Will fix later...
			Renderer::Vulkan::RenderSystem::DestroyVulkanDebug(true);
		}

		void RenderSystem::InitVulkanDebug(bool enableValidation)
		{
			if (enableValidation)
			{
				VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT
					| VK_DEBUG_REPORT_WARNING_BIT_EXT
					//| VK_DEBUG_REPORT_INFORMATION_BIT_EXT
					| VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT;
					//| VK_DEBUG_REPORT_DEBUG_BIT_EXT;

				vkDebug::setupDebugging(vkInstance, debugReportFlags, VK_NULL_HANDLE);
			}
		}

		void RenderSystem::DestroyVulkanDebug(bool enableValidation)
		{
			if (enableValidation)
			{
				vkDebug::freeDebugCallback(vkInstance);
			}
		}

		void RenderSystem::InitVulkanDevice(bool benableValidation)
		{
			uint32_t deviceCount = 0;
			VkResult result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, nullptr);
			if (result != VK_SUCCESS)
			{
				fprintf(stderr, "Failed to query the number of physical devices present: %d\n", result);
				return;
			}

			// There has to be at least one device present
			if (deviceCount == 0)
			{
				fprintf(stderr, "Couldn't detect any device present with Vulkan support: %d\n", result);
				return;
			}

			std::vector<VkPhysicalDevice> physicalDevices;
			physicalDevices.resize(deviceCount);

			// Get the physical devices enumeration properties
			result = vkEnumeratePhysicalDevices(vkInstance, &deviceCount, physicalDevices.data());
			if (result != VK_SUCCESS)
			{
				fprintf(stderr, "Faied to enumerate physical devices present: %d\n", result);
				return;
			}


			// Enumerate all physical devices
			VkPhysicalDeviceProperties deviceProperties;
			VkPhysicalDeviceFeatures deviceFeatures;

			for (uint32_t i = 0; i < deviceCount; i++)
			{
				memset(&deviceProperties, 0, sizeof deviceProperties);
				vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);

				printf("Driver Version: %d\n", deviceProperties.driverVersion);
				printf("Device Name:    %s\n", deviceProperties.deviceName);
				printf("Device Type:    %d\n", deviceProperties.deviceType);
				printf("API Version:    %d.%d.%d\n",
					// See note regarding this:
					VK_VER_MAJOR(deviceProperties.apiVersion),
					VK_VER_MINOR(deviceProperties.apiVersion),
					VK_VER_PATCH(deviceProperties.apiVersion));
			}

			//Use always firt found device
			vkPhysicalDevice = physicalDevices[0];

			// Get physical device memory properties and features
			vkGetPhysicalDeviceMemoryProperties(vkPhysicalDevice, &vkPhysicalDeviceMemoryProperties);
			vkGetPhysicalDeviceFeatures(vkPhysicalDevice, &deviceFeatures);


			uint32_t queueFamilyCount = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, nullptr);

			std::vector<VkQueueFamilyProperties> queueFamilyProperties;
			queueFamilyProperties.resize(queueFamilyCount);
			vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice, &queueFamilyCount, queueFamilyProperties.data());


			// Get the graphics queue
			vkGraphicsQueueFamilyIndex = 0u;
			for (uint32_t j = 0; j < queueFamilyCount; j++)
			{
				printf("Count of Queues: %d\n", queueFamilyProperties[j].queueCount);
				printf("Supported operationg on this queue:\n");

				if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					printf("\t\t Graphics\n");
					vkGraphicsQueueFamilyIndex = j;
				}
				if (queueFamilyProperties[j].queueFlags & VK_QUEUE_COMPUTE_BIT)
				{
					printf("\t\t Compute\n");
				}
				if (queueFamilyProperties[j].queueFlags & VK_QUEUE_TRANSFER_BIT)
				{
					printf("\t\t Transfer\n");
				}
				if (queueFamilyProperties[j].queueFlags & VK_QUEUE_SPARSE_BINDING_BIT)
				{
					printf("\t\t Sparse Binding\n");
				}
			}

			// Setup device queue
			const float queuePriorities[1] = { 0.0f };
			VkDeviceQueueCreateInfo queueCreateInfo = {};
			{
				queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
				queueCreateInfo.queueFamilyIndex = vkGraphicsQueueFamilyIndex;
				queueCreateInfo.queueCount = 1u;
				queueCreateInfo.pQueuePriorities = queuePriorities;
			}

			VkDeviceCreateInfo deviceCreateInfo = {};
			deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			deviceCreateInfo.pNext = NULL;
			deviceCreateInfo.queueCreateInfoCount = 1;
			deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
			deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

			std::vector<const char*> enabledExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

			// enable the debug marker extension if it is present (likely meaning a debugging tool is present)
			if (VkTools::CheckDeviceExtensionPresent(vkPhysicalDevice, VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
			{
				enabledExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
				//enableDebugMarkers = true;
			}

			if (enabledExtensions.size() > 0)
			{
				deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(enabledExtensions.size());
				deviceCreateInfo.ppEnabledExtensionNames = enabledExtensions.data();
			}

			VK_CHECK_RESULT(vkCreateDevice(vkPhysicalDevice, &deviceCreateInfo, nullptr, &vkDevice));

			//Get graphics	queue
			vkGetDeviceQueue(vkDevice, vkGraphicsQueueFamilyIndex, 0, &vkQueue);

			if (benableValidation)
			{
				vkDebug::DebugMarker::setup(vkDevice);
			}
		}

		void RenderSystem::InitCommandPool()
		{
			//Primary command pool
			{
				VkCommandPoolCreateInfo cmdPoolInfo = {};
				cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
				cmdPoolInfo.pNext = nullptr;
				cmdPoolInfo.queueFamilyIndex = vkGraphicsQueueFamilyIndex;
				cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

				VK_CHECK_RESULT(vkCreateCommandPool(vkDevice, &cmdPoolInfo, nullptr, &vkPrimalCommandPool));
			}

			//Secondary command pool
			{
				uint32_t secondary_command_buffer_count = SECONDARY_COMMAND_BUFFER_COUNT;
				vkSecondaryCommandPools.resize(secondary_command_buffer_count);

				for (uint32_t i = 0; i < secondary_command_buffer_count; i++)
				{
					VkCommandPoolCreateInfo cmdPoolInfo = {};
					cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
					cmdPoolInfo.pNext = nullptr;
					cmdPoolInfo.queueFamilyIndex = vkGraphicsQueueFamilyIndex;
					cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

					VK_CHECK_RESULT(vkCreateCommandPool(vkDevice, &cmdPoolInfo, nullptr, &vkSecondaryCommandPools[i]));
				}
			}
		}

		void RenderSystem::InitCommandBuffers()
		{
			{
				uint32_t primal_command_buffer_count = vkSwapchainImages.size();
				vkPrimalCommandBuffers.resize(primal_command_buffer_count);

				VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(vkPrimalCommandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, primal_command_buffer_count);
				VK_CHECK_RESULT(vkAllocateCommandBuffers(vkDevice, &cmdBufAllocateInfo, vkPrimalCommandBuffers.data()));
			}

			{
				uint32_t secondary_command_buffer_count = vkSwapchainImages.size() * SECONDARY_COMMAND_BUFFER_COUNT; 
				vkSecondaryCommandBuffers.resize(secondary_command_buffer_count);

				for (uint32_t i = 0; i < secondary_command_buffer_count; i++)
				{
					VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(vkSecondaryCommandPools[i % SECONDARY_COMMAND_BUFFER_COUNT], VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1);
					VK_CHECK_RESULT(vkAllocateCommandBuffers(vkDevice, &cmdBufAllocateInfo, &vkSecondaryCommandBuffers[i]));
				}
			}
		}

		void RenderSystem::DestroyCommandBuffers()
		{
			//Destroy primary command buffer
			vkFreeCommandBuffers(vkDevice, vkPrimalCommandPool, vkPrimalCommandBuffers.size(), vkPrimalCommandBuffers.data());
			vkPrimalCommandBuffers.clear();

			//Destroy secondary command buffer
			uint32_t secondary_command_buffer_count = vkSwapchainImages.size() * SECONDARY_COMMAND_BUFFER_COUNT;

			for(uint32_t i = 0; i < secondary_command_buffer_count; i++)
			{
				vkFreeCommandBuffers(vkDevice, vkSecondaryCommandPools[i % SECONDARY_COMMAND_BUFFER_COUNT], 1u, &vkSecondaryCommandBuffers[i]);
			}
			vkSecondaryCommandBuffers.clear();
		}

		void RenderSystem::Init(
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
		)
		{
			//Allocate resources
			Renderer::Resource::RenderPassManager::init();
			Renderer::Resource::PipelineLayoutManager::init();
			Renderer::Resource::GpuProgramManager::init();
			Renderer::Resource::PipelineManager::init();
			Renderer::Resource::BufferLayoutManager::init();
			Renderer::Resource::BufferObjectManager::init();
			Renderer::Resource::ImageManager::init();
			Renderer::Resource::FrameBufferManager::init();
			Renderer::Resource::DrawCallManager::init();
			Renderer::Vulkan::GpuMemoryManager::Init();

			InitVulkanInstance(enableValidation, application_name);
			InitVulkanDebug(enableValidation);
			InitVulkanDevice(enableValidation);
			InitPlatformDependentFormats();
			InitVulkanSurface(handle, window);
			InitOrUpdateVulkanSwapChain(enableVsync);
			InitVulkanSynchronization();
			InitCommandPool();
			InitCommandBuffers();
			InitVulkanPipelineCache();
		}

		void RenderSystem::InitVulkanSurface(
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
		)
		{
			VkResult result;

			// Create surface depending on OS
#ifdef _WIN32
			VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.hinstance = handle;
			surfaceCreateInfo.hwnd = window;
			result = vkCreateWin32SurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &vkSurface);
#else
#ifdef __ANDROID__
			VkAndroidSurfaceCreateInfoKHR surfaceCreateInfo = {};
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.window = window;
			result = vkCreateAndroidSurfaceKHR(vkInstance, &surfaceCreateInfo, NULL, &vkSurface);
#else
			VkXcbSurfaceCreateInfoKHR surfaceCreateInfo = {};
			surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_XCB_SURFACE_CREATE_INFO_KHR;
			surfaceCreateInfo.connection = connection;
			surfaceCreateInfo.window = window;
			result = vkCreateXcbSurfaceKHR(vkInstance, &surfaceCreateInfo, nullptr, &vkSurface);
#endif
#endif
			VK_CHECK_RESULT(result);
		}

		void RenderSystem::InitOrUpdateVulkanSwapChain(bool vsync)
		{
			// Get physical device surface properties and formats
			VkBool32 supported = false;
			VkResult result = vkGetPhysicalDeviceSurfaceSupportKHR(vkPhysicalDevice, 0u, vkSurface, &supported);

			VK_CHECK_RESULT(result);
			assert(supported);

			// Get surfaces "features"
			VkSurfaceCapabilitiesKHR surfaceCapabilities;
			result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vkPhysicalDevice, vkSurface, &surfaceCapabilities);
			VK_CHECK_RESULT(result);

			static VkFormat surfaceFormatToUse = VK_FORMAT_B8G8R8A8_UNORM;
			static VkColorSpaceKHR surfaceColorSpaceToUse = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			// Get available present modes
			uint32_t presentModeCount;
			result = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, NULL);
			VK_CHECK_RESULT(result);
			assert(presentModeCount > 0);

			std::vector<VkPresentModeKHR> presentModes;
			presentModes.resize(presentModeCount);

			result = vkGetPhysicalDeviceSurfacePresentModesKHR(vkPhysicalDevice, vkSurface, &presentModeCount, presentModes.data());
			VK_CHECK_RESULT(result);


			// Select a present mode for the swapchain

			// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
			// This mode waits for the vertical blank ("v-sync")
			VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

			// If v-sync is not requested, try to find a mailbox mode if present
			// It's the lowest latency non-tearing present mode available
			if (!vsync)
			{
				for (size_t i = 0; i < presentModeCount; i++)
				{
					std::cout << "Present mode " << presentModes[i] << " available\n";

					if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
					{
						swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					}
					if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
					{
						swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
					}
				}
			}

			// Check if surface format is supported
			{
				uint32_t formatCount = 0u;
				result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, nullptr);
				VK_CHECK_RESULT(result);

				assert(formatCount > 0u);
				std::vector<VkSurfaceFormatKHR> surfaceFormats;
				surfaceFormats.resize(formatCount);

				result = vkGetPhysicalDeviceSurfaceFormatsKHR(vkPhysicalDevice, vkSurface, &formatCount, surfaceFormats.data());
				VK_CHECK_RESULT(result);

				bool found = false;
				for (uint32_t i = 0u; i < formatCount; ++i)
				{
					if (surfaceFormats[i].colorSpace == surfaceColorSpaceToUse &&
						surfaceFormats[i].format == surfaceFormatToUse)
					{
						found = true;
						break;
					}
				}
				assert(found &&"Surface format and/or surface color space not supported");
			}

			VkSwapchainCreateInfoKHR swapchainCI = {};
			swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
			swapchainCI.pNext = nullptr;
			swapchainCI.surface = vkSurface;
			swapchainCI.minImageCount = std::min(2u, surfaceCapabilities.maxImageCount);
			swapchainCI.imageFormat = surfaceFormatToUse;
			swapchainCI.imageColorSpace = surfaceColorSpaceToUse;
			swapchainCI.imageExtent = surfaceCapabilities.currentExtent;
			swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			swapchainCI.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
			swapchainCI.imageArrayLayers = 1;
			swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			swapchainCI.queueFamilyIndexCount = 0;
			swapchainCI.pQueueFamilyIndices = nullptr;
			swapchainCI.presentMode = swapchainPresentMode;
			swapchainCI.oldSwapchain = vkSwapchain;
			swapchainCI.clipped = true;
			swapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

			result = vkCreateSwapchainKHR(vkDevice, &swapchainCI, nullptr, &vkSwapchain);
			VK_CHECK_RESULT(result);

			// If an existing sawp chain is re-created, destroy the old swap chain
			// This also cleans up all the presentable images
			if (swapchainCI.oldSwapchain != VK_NULL_HANDLE)
			{
				for (uint32_t i = 0; i < vkSwapchainImageViews.size(); i++)
				{
					vkDestroyImageView(vkDevice, vkSwapchainImageViews[i], nullptr);
				}
				vkDestroySwapchainKHR(vkDevice, swapchainCI.oldSwapchain, nullptr);
			}

			//Retrieve swapchain images
			uint32_t swapchainImageCount = 0u;
			{
				result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapchainImageCount, NULL);
				VK_CHECK_RESULT(result);

				// Get the swap chain images
				vkSwapchainImages.resize(swapchainImageCount);
				vkSwapchainImageViews.resize(swapchainImageCount);

				result = vkGetSwapchainImagesKHR(vkDevice, vkSwapchain, &swapchainImageCount, vkSwapchainImages.data());
				VK_CHECK_RESULT(result);
			}

			// Get the swap chain buffers containing the image and imageview
			for (uint32_t i = 0; i < swapchainImageCount; i++)
			{
				VkImageViewCreateInfo colorAttachmentView = {};
				colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
				colorAttachmentView.pNext = NULL;
				colorAttachmentView.format = surfaceFormatToUse;
				colorAttachmentView.components = {
					VK_COMPONENT_SWIZZLE_R,
					VK_COMPONENT_SWIZZLE_G,
					VK_COMPONENT_SWIZZLE_B,
					VK_COMPONENT_SWIZZLE_A
				};
				colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				colorAttachmentView.subresourceRange.baseMipLevel = 0;
				colorAttachmentView.subresourceRange.levelCount = 1;
				colorAttachmentView.subresourceRange.baseArrayLayer = 0;
				colorAttachmentView.subresourceRange.layerCount = 1;
				colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
				colorAttachmentView.flags = 0;
				colorAttachmentView.image = vkSwapchainImages[i];

				auto const viewInfo = vk::ImageViewCreateInfo()
					.setImage(vkSwapchainImages[i])
					.setViewType(vk::ImageViewType::e2D)
					.setFormat(vk::Format::eB8G8R8A8Unorm)
					.setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

				result = vkCreateImageView(vkDevice, reinterpret_cast<const VkImageViewCreateInfo*>(&viewInfo), nullptr, &vkSwapchainImageViews[i]);
				VK_CHECK_RESULT(result);
			}

			std::vector<DOD::Ref> imagesToCreate;
			backBufferDimensions = glm::uvec2(swapchainCI.imageExtent.width, swapchainCI.imageExtent.height);

			std::vector<DOD::Ref> imagesToDestroy;

			//Destroy existing backbuffer
			for (int i = 0; i < swapchainImageCount; i++)
			{
				std::string bufferName = "Backbuffer" + std::to_string(i);
				DOD::Ref imageRef = Renderer::Resource::ImageManager::GetResourceByName(bufferName);

				if (imageRef.isValid())
				{
					imagesToDestroy.push_back(imageRef);
				}
				else
				{
					Renderer::Resource::ImageManager::CreateImage(bufferName);
				}
			}

			Renderer::Resource::ImageManager::DestroyResource(imagesToDestroy);

			//Create backbuffer resource
			for (int i = 0; i < swapchainImageCount; i++)
			{
				std::string bufferName = "Backbuffer" + std::to_string(i);
				DOD::Ref backBufferRef = Renderer::Resource::ImageManager::GetResourceByName(bufferName);

				Renderer::Resource::ImageManager::GetVkImage(backBufferRef) = vkSwapchainImages[i];
				Renderer::Resource::ImageManager::ResetToDefault(backBufferRef);

				Renderer::Resource::ImageManager::GetVkImage(backBufferRef) = vkSwapchainImages[i];
				Renderer::Resource::ImageManager::GetImageView(backBufferRef) = vkSwapchainImageViews[i];
				Renderer::Resource::ImageManager::GetSubresourceImageViews(backBufferRef).resize(1u);
				Renderer::Resource::ImageManager::GetSubresourceImageViews(backBufferRef)[0u].resize(1u);
				Renderer::Resource::ImageManager::GetSubresourceImageViews(backBufferRef)[0u][0u] = vkSwapchainImageViews[i];
				Renderer::Resource::ImageManager::GetImageDimensions(backBufferRef) = glm::uvec3(backBufferDimensions, 1u);
				Renderer::Resource::ImageManager::AddImageFlags(backBufferRef,
					                                            ImageFlags::kExternalImage |
					                                            ImageFlags::kExternalView |
					                                            ImageFlags::kExternalDeviceMemory);

				imagesToCreate.push_back(backBufferRef);
			}

			//Renderer::Resource::ImageManager::CreateResource(imagesToCreate);
		}

		void RenderSystem::InitVulkanPipelineCache()
		{
			VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
			{
				pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
				pipelineCacheCreateInfo.pNext = nullptr;
				pipelineCacheCreateInfo.initialDataSize = 0;
				pipelineCacheCreateInfo.pInitialData = nullptr;
				pipelineCacheCreateInfo.flags = 0u;
			};

			VK_CHECK_RESULT(vkCreatePipelineCache(vkDevice, &pipelineCacheCreateInfo, nullptr, &vkPipelineCache));
		}

		void RenderSystem::InitPlatformDependentFormats()
		{
			VkBool32 validDepthFormat = VkTools::GetSupportedDepthFormat(vkPhysicalDevice, vkDepthFormatToUse);
			assert(validDepthFormat && "Failed to find valid depth format");
		}

		void RenderSystem::InitVulkanSynchronization()
		{
			VkSemaphoreCreateInfo imageAcquiredSemaphoreCreateInfo;
			imageAcquiredSemaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
			imageAcquiredSemaphoreCreateInfo.pNext = nullptr;
			imageAcquiredSemaphoreCreateInfo.flags = 0;

			VkResult result = vkCreateSemaphore(vkDevice, &imageAcquiredSemaphoreCreateInfo, nullptr, &vkImageAcquireSemaphore);
			VK_CHECK_RESULT(result);

			vkDrawFences.resize(vkSwapchainImages.size());
			for (uint32_t i = 0u; i < vkSwapchainImages.size(); ++i)
			{
				VkFenceCreateInfo fenceInfo;
				fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
				fenceInfo.pNext = nullptr;
				fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
				VkResult result = vkCreateFence(vkDevice, &fenceInfo, nullptr, & vkDrawFences[i]);
				VK_CHECK_RESULT(result);
			}

			result = vkResetFences(vkDevice, vkDrawFences.size(), vkDrawFences.data());
			VK_CHECK_RESULT(result);
		}

		void RenderSystem::ResizeSwapchain()
		{
			vkDeviceWaitIdle(vkDevice);
			InitOrUpdateVulkanSwapChain(true);

			Renderer::Resource::DrawCallManager::DestroyDrawCallsAndResources(Renderer::Resource::DrawCallManager::activeRefs);
			Renderer::Resource::PipelineLayoutManager::DestroyPipelineLayoutAndResources(Renderer::Resource::PipelineLayoutManager::activeRefs);
			Renderer::Resource::PipelineManager::DestroyPipelineAndResources(Renderer::Resource::PipelineManager::activeRefs);
			Renderer::Resource::FrameBufferManager::DestroyFrameBufferAndResources(Renderer::Resource::FrameBufferManager::activeRefs);
			Renderer::Resource::RenderPassManager::DestroyRenderPassAndResources(Renderer::Resource::FrameBufferManager::activeRefs);
		}

		void RenderSystem::StartFrame()
		{
			VkResult result = vkAcquireNextImageKHR(vkDevice, vkSwapchain, UINT64_MAX, vkImageAcquireSemaphore, VK_NULL_HANDLE, &backBufferIndex);
			VK_CHECK_RESULT(result);

			WaitForFrame(backBufferIndex);
			BeginPrimaryCommandBuffer();
			InsertPostPresentBarrier();
		}

		void RenderSystem::EndFrame()
		{
			//Wait for all rendering tasks to finnish here
			//Scheduler.wait_for_all_tasks

			InsertPrePresentBarrier();
			EndPrimaryCommandBuffer();

			{
				VkPipelineStageFlags pipeStageFlags = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
				VkSubmitInfo submitInfo = {};
				{
					submitInfo.pNext = nullptr;
					submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
					submitInfo.waitSemaphoreCount = 1u;
					submitInfo.pWaitSemaphores = &vkImageAcquireSemaphore;
					submitInfo.pWaitDstStageMask = &pipeStageFlags;
					submitInfo.commandBufferCount = 1u;
					submitInfo.pCommandBuffers = &vkPrimalCommandBuffers[backBufferIndex];
					submitInfo.signalSemaphoreCount = 0u;
					submitInfo.pSignalSemaphores = nullptr;
				}

				VkResult result = vkQueueSubmit(vkQueue, 1u, &submitInfo, vkDrawFences[backBufferIndex]);
				VK_CHECK_RESULT(result);
			}

			activeBackbufferMask |= 1u << backBufferIndex;

			{
				VkPresentInfoKHR present = {};
				{
					present.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
					present.pNext = nullptr;
					present.swapchainCount = 1u;
					present.pSwapchains = &vkSwapchain;
					present.pImageIndices = &backBufferIndex;
					present.pWaitSemaphores = nullptr;
					present.waitSemaphoreCount = 0u;
					present.pResults = nullptr;
				}

				VkResult result = vkQueuePresentKHR(vkQueue, &present);
				VK_CHECK_RESULT(result);
			}
		}

		bool RenderSystem::WaitForFrame(const uint32_t index)
		{
			if ((activeBackbufferMask & (1u << index)) > 0u)
			{
				VkResult result = VkResult::VK_TIMEOUT;

				do
				{
					result = vkWaitForFences(vkDevice, 1u, &vkDrawFences[index], VK_TRUE, UINT64_MAX);
				} while (result == VK_TIMEOUT);
				VK_CHECK_RESULT(result);

				result = vkResetFences(vkDevice, 1u, &vkDrawFences[index]);
				VK_CHECK_RESULT(result);

				activeBackbufferMask &= ~(1u << index);
				return true;
			}
			return false;
		}

		void RenderSystem::BeginPrimaryCommandBuffer()
		{
			VkCommandBufferBeginInfo cmdBufBeginInfo = {};
			{
				cmdBufBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				cmdBufBeginInfo.pNext = nullptr;
				cmdBufBeginInfo.flags = 0u;
				cmdBufBeginInfo.pInheritanceInfo = nullptr;
			}

			VkResult result = vkBeginCommandBuffer(vkPrimalCommandBuffers[backBufferIndex], &cmdBufBeginInfo);
			VK_CHECK_RESULT(result);
		}

		void RenderSystem::EndPrimaryCommandBuffer()
		{
			VkResult result = vkEndCommandBuffer(vkPrimalCommandBuffers[backBufferIndex]);
			VK_CHECK_RESULT(result);
		}

	    void RenderSystem::BeginSecondaryComandBuffer(uint32_t p_CmdBufferIdx, VkRenderPass p_VkRenderPass, VkFramebuffer p_VkFramebuffer)
		{
			VkCommandBufferInheritanceInfo inheritanceInfo = {};
			{
				inheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
				inheritanceInfo.pNext = nullptr;
				inheritanceInfo.renderPass = p_VkRenderPass;
				inheritanceInfo.framebuffer = p_VkFramebuffer;
			}

			VkCommandBufferBeginInfo commandBufferBeginInfo = {};
			{
				commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
				commandBufferBeginInfo.pNext = nullptr;
				commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
				commandBufferBeginInfo.pInheritanceInfo = &inheritanceInfo;
			}

			VK_CHECK_RESULT(vkBeginCommandBuffer(
				vkSecondaryCommandBuffers[backBufferIndex * SECONDARY_COMMAND_BUFFER_COUNT + p_CmdBufferIdx], &commandBufferBeginInfo));
		}

		void RenderSystem::EndSecondaryComandBuffer(uint32_t p_CmdBufferId)
		{
			VK_CHECK_RESULT(vkEndCommandBuffer
			(vkSecondaryCommandBuffers[backBufferIndex * SECONDARY_COMMAND_BUFFER_COUNT + p_CmdBufferId]));
		}

		void RenderSystem::BeginRenderPass(const DOD::Ref& renderPass, const DOD::Ref& frameBufferRef, VkSubpassContents p_SubpassContents, uint32_t clearValueCount, VkClearValue* p_ClearValues)
		{
			VkRenderPassBeginInfo renderPassBegin = {};
			{
				renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				renderPassBegin.pNext = nullptr;
				renderPassBegin.renderPass = Resource::RenderPassManager::GetRenderPass(renderPass);
				renderPassBegin.framebuffer = Resource::FrameBufferManager::GetFrameBuffer(frameBufferRef);

				const glm::uvec2& framebufferDim = Resource::FrameBufferManager::GetDimensions(frameBufferRef);
				renderPassBegin.renderArea.offset.x = 0u;
				renderPassBegin.renderArea.offset.y = 0u;
				renderPassBegin.renderArea.extent.width = framebufferDim.x;
				renderPassBegin.renderArea.extent.height = framebufferDim.y;

				renderPassBegin.clearValueCount = clearValueCount;
				renderPassBegin.pClearValues = p_ClearValues;
			}
			
			vkCmdBeginRenderPass(GetPrimaryCommandBuffer(), &renderPassBegin, p_SubpassContents);
		}

		void RenderSystem::InsertPostPresentBarrier()
		{
			VkCommandBuffer vkCmdBuffer = GetPrimaryCommandBuffer();

			VkImageMemoryBarrier postPresentBarrier = {};
			{
				postPresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				postPresentBarrier.pNext = nullptr;
				postPresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
				postPresentBarrier.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				postPresentBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				postPresentBarrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				postPresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				postPresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				postPresentBarrier.subresourceRange.aspectMask =
					VK_IMAGE_ASPECT_COLOR_BIT;
				postPresentBarrier.subresourceRange.baseMipLevel = 0u;
				postPresentBarrier.subresourceRange.levelCount = 1u;
				postPresentBarrier.subresourceRange.baseArrayLayer = 0u;
				postPresentBarrier.subresourceRange.layerCount = 1u;
				postPresentBarrier.image = vkSwapchainImages[backBufferIndex];

				vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &postPresentBarrier);
			}
		}

		void RenderSystem::InsertPrePresentBarrier()
		{
			VkCommandBuffer vkCmdBuffer = GetPrimaryCommandBuffer();

			VkImageMemoryBarrier prePresentBarrier = {};
			{
				prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				prePresentBarrier.pNext = nullptr;
				prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				prePresentBarrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				prePresentBarrier.subresourceRange.baseMipLevel = 0u;
				prePresentBarrier.subresourceRange.levelCount = 1u;
				prePresentBarrier.subresourceRange.baseArrayLayer = 0u;
				prePresentBarrier.subresourceRange.layerCount = 1u;
				prePresentBarrier.image = vkSwapchainImages[backBufferIndex];

				vkCmdPipelineBarrier(vkCmdBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, nullptr, 0, nullptr, 1, &prePresentBarrier);
			}
		}
	}
}
