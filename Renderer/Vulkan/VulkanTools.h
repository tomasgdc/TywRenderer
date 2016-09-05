#pragma once

#include <cstdint>
#include <External\vulkan\vulkan.h>



// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000


#define VK_CHECK_RESULT(vkRes)																		\
	{																								 \
		if (vkRes != VK_SUCCESS)																	 \
		{																							 \
			fprintf(stdout, "Fatal : VkResult is %s in %s at line %i \n", VkTools::VkResultToString(vkRes).c_str(), __FILE__, __LINE__);	 \
			assert(vkRes == VK_SUCCESS);															 \
		}																							 \
	}																								 \


// Synchronization semaphores
struct VkSemaphores
{
	// Swap chain image presentation
	VkSemaphore presentComplete;

	// Command buffer submission and execution
	VkSemaphore renderComplete;

	// Text overlay submission and execution
	VkSemaphore textOverlayComplete;
};


struct VkDepthStencil
{
	VkImage image;
	VkDeviceMemory mem;
	VkImageView view;
};


namespace VkTools
{
	//Objects
	// Contains all vulkan objects
	// required for a uniform data object
	struct UniformData
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
		uint32_t allocSize;
		void* mapped = nullptr;
	};

	TYWRENDERER_API std::string VkResultToString(VkResult errorCode);

	TYWRENDERER_API void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);

	TYWRENDERER_API uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, VkPhysicalDeviceMemoryProperties& deviceMemoryProperties);

	// Create an image memory barrier for changing the layout of
	// an image and put it into an active command buffer
	// See chapter 11.4 "Image Layout" for details
	TYWRENDERER_API void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);


	TYWRENDERER_API void ExitFatal(std::string message, std::string caption);

	// Check if extension is present on the given device
	TYWRENDERER_API VkBool32 CheckDeviceExtensionPresent(VkPhysicalDevice physicalDevice, const char* extensionName);

	// Selected a suitable supported depth format starting with 32 bit down to 16 bit
	// Returns false if none of the depth formats in the list is supported by the device
	TYWRENDERER_API VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat);

	//Destroys uniform data that was allocated
	TYWRENDERER_API void DestroyUniformData(VkDevice device, UniformData *uniformData);


#if defined(__ANDROID__)
	// Android shaders are stored as assets in the apk
	// So they need to be loaded via the asset manager
	VkShaderModule LoadShader(AAssetManager* assetManager, const char *fileName, VkDevice device, VkShaderStageFlagBits stage);
#else
	TYWRENDERER_API VkShaderModule LoadShader(const std::string& fileName, VkDevice device, VkShaderStageFlagBits stage);
#endif

	namespace Initializer
	{
		TYWRENDERER_API VkSemaphoreCreateInfo SemaphoreCreateInfo();
		TYWRENDERER_API VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags);
		TYWRENDERER_API VkEventCreateInfo EventCreateInfo();

		TYWRENDERER_API VkSubmitInfo SubmitInfo();
		TYWRENDERER_API VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t bufferCount);
		TYWRENDERER_API VkCommandBufferBeginInfo CommandBufferBeginInfo();

		TYWRENDERER_API VkImageMemoryBarrier ImageMemoryBarrier();
		TYWRENDERER_API VkMemoryAllocateInfo MemoryAllocateInfo();

		TYWRENDERER_API VkBufferCreateInfo BufferCreateInfo();
		TYWRENDERER_API VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage,VkDeviceSize size);

		TYWRENDERER_API VkImageCreateInfo ImageCreateInfo();
		TYWRENDERER_API VkSamplerCreateInfo SamplerCreateInfo();
		TYWRENDERER_API VkImageViewCreateInfo ImageViewCreateInfo();

		TYWRENDERER_API VkRenderPassCreateInfo RenderPassCreateInfo();
		TYWRENDERER_API VkFramebufferCreateInfo FramebufferCreateInfo();
		TYWRENDERER_API VkRenderPassBeginInfo RenderPassBeginInfo();


		TYWRENDERER_API VkViewport Viewport(float width, float height, float minDepth, float maxDepth);
		TYWRENDERER_API VkRect2D Rect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY);

		TYWRENDERER_API VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(const VkDynamicState * pDynamicStates, uint32_t dynamicStateCount, VkPipelineDynamicStateCreateFlags flags);

		TYWRENDERER_API VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type,VkShaderStageFlags stageFlags,uint32_t binding);
		TYWRENDERER_API VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* pBindings,uint32_t bindingCount);
		TYWRENDERER_API VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,uint32_t setLayoutCount);
		TYWRENDERER_API VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,const VkDescriptorSetLayout* pSetLayouts,uint32_t descriptorSetCount);
		TYWRENDERER_API VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets);
		TYWRENDERER_API VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount);

		TYWRENDERER_API VkWriteDescriptorSet  WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		TYWRENDERER_API VkWriteDescriptorSet  WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo * imageInfo);
		TYWRENDERER_API VkVertexInputBindingDescription  VertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);
		TYWRENDERER_API VkVertexInputAttributeDescription  VertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
		TYWRENDERER_API VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);
	}
}
