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





namespace VkTools
{
	/*
		Objects
		Contains all vulkan object
		required for a uniform data object
	*/
	struct UniformData
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
		uint32_t allocSize;
		void* mapped = nullptr;
	};

	struct VkDepthStencil
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};

	struct FrameBufferAttachment 
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
	};

	struct FrameBuffer
	{
		int32_t width, height;
		VkFramebuffer frameBuffer;
		VkRenderPass renderPass;
	};

	/*
		Creates ImageView that will be used in render pass
		Check 
		https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#_framebuffers
		https://www.khronos.org/registry/vulkan/specs/1.0/html/vkspec.html#resources-image-views

		@param: uint32_t iWidth
		@param: uint32_t iHeight
		@param: VkFormat format
		@param: VkImageUsageFlagBits usage
		@param: FrameBufferAttachment *attachment - new data will be stored in this one
		@param: VkCommandBuffer layoutCmd
		@param: VkDevice device
		@param: VkPhysicalDeviceMemoryProperties& deviceMemoryProperties
	*/
	 void CreateFrameBufferAttachement(uint32_t iWidth, uint32_t iHeight, VkFormat format, VkImageUsageFlagBits usage,FrameBufferAttachment *attachment,VkCommandBuffer layoutCmd,VkDevice device,VkPhysicalDeviceMemoryProperties& deviceMemoryProperties);

	/*
		@param: VkResult errorCode
		@return: std::string
	*/
	 std::string VkResultToString(const VkResult& errorCode);

	/*
		@param: VkCommandBuffer cmdbuffer
		@param: VkImage image
		@param: VkImageAspectFlags aspectMask
		@param: VkImageLayout oldImageLayout
		@param: VkImageLayout newImageLayout
	*/
	 void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout);


	/*
		@param: uint32_t typeBit
		@param: VkFlags properties
		@parma: VkPhysicalDeviceMemoryProperties& deviceMemoryProperties

		@return: uint32_t
	*/
	 uint32_t GetMemoryType(uint32_t typeBits, VkFlags properties, VkPhysicalDeviceMemoryProperties& deviceMemoryProperties);

	/*	
		Create an image memory barrier for changing the layout of
		an image and put it into an active command buffer
		See chapter 11.4 "Image Layout" for details

		@param: VkCommandBuffer cmdbuffer
		@param: VkImage image
		@param: VkImageAspectFlags aspectMask
		@param: VkImageLayout oldImageLayout
		@param: VkImageLayout newImageLayout
		@param: VkImageSubresourceRange subresourceRange
	*/
	 void SetImageLayout(VkCommandBuffer cmdbuffer, VkImage image, VkImageAspectFlags aspectMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkImageSubresourceRange subresourceRange);

	/*
		@param: const std::string& message
		@param: const std::string& caption
	*/
	 void ExitFatal(const std::string& message, const std::string& caption);

	/*
		Check if extension is present on the given device

		@param: VkPhysicalDevice physicalDevice
		@param: const char* extensionName

		@return VkBool32
	*/
	 VkBool32 CheckDeviceExtensionPresent(VkPhysicalDevice physicalDevice, const char* extensionName);

	/*
		Selected a suitable supported depth format starting with 32 bit down to 16 bit
		Returns false if none of the depth formats in the list is supported by the device

		@param: VkPhysicalDevice physicalDevice
		@param: VkFormat& depthFormat
		
		@return VkBool32
	*/
	 VkBool32 GetSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat& depthFormat);

	/*
		Destroys uniform data that was allocated

		@param: VkDevice device
		@param: UniformData& uniformData
	*/
	 void DestroyUniformData(VkDevice device, UniformData& uniformData);


#if defined(__ANDROID__)

	/*
		Android shaders are stored as assets in the apk 
		So they need to be loaded via the asset manager
		@param: AAssetManager* assetManager
		@param: const char *fileName
		@param: VkDevice device
		@param: VkShaderStageFlagBits stage

		@return VkShaderModule
	*/
	VkShaderModule LoadShader(AAssetManager* assetManager, const char *fileName, VkDevice device, VkShaderStageFlagBits stage);
#else
	/*
		@param: const std::string& fileName
		@param: VkDevice device
		@param: VkShaderStageFlagBits stage

		@return VkShaderModule
	*/
	 VkShaderModule LoadShader(const std::string& fileName, VkDevice device, VkShaderStageFlagBits stage);
#endif

	namespace Initializer
	{
		/*
			@return VkSemaphoreCreateInfo
		*/
		 VkSemaphoreCreateInfo SemaphoreCreateInfo();

		/*
			@param: VkFenceCreateFlags flags
			@return VkFenceCreateInfo;
		*/
		 VkFenceCreateInfo FenceCreateInfo(VkFenceCreateFlags flags = VK_FLAGS_NONE);

		/*
			@return: VkEventCreateInfo;
		*/
		 VkEventCreateInfo EventCreateInfo();

		/*
			@return: VkSubmitInfo
		*/
		 VkSubmitInfo SubmitInfo();

		/*
			@param: VkCommandPool commandPool
			@param: VkCommandBufferLevel level
			@param: uint32_t bufferCount

			@return: VkCommandBufferAllocateInfo
		*/
		 VkCommandBufferAllocateInfo CommandBufferAllocateInfo(VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t bufferCount);

		/*
			@return: VkCommandBufferBeginInfo
		*/
		 VkCommandBufferBeginInfo CommandBufferBeginInfo();

		/*
			@return: VkImageMemoryBarrier
		*/
		 VkImageMemoryBarrier ImageMemoryBarrier();

		/*
			@return: VkMemoryAllocateInfo
		*/
		 VkMemoryAllocateInfo MemoryAllocateInfo();

		/*
			@return: VkBufferCreateInfo
		*/
		 VkBufferCreateInfo BufferCreateInfo();

		/*
			@param: VkBufferUsageFlags usage
			@param: VkDeviceSize size

			@return: VkBufferCreateInfo
		*/
		 VkBufferCreateInfo BufferCreateInfo(VkBufferUsageFlags usage,VkDeviceSize size);

		 /*
			@param:
			@param:
			@param:

			@return
		 */
		 VkPushConstantRange PushConstantRange(VkShaderStageFlags stageFlags, uint32_t size, uint32_t offset);

		 VkImageCreateInfo ImageCreateInfo();
		 VkSamplerCreateInfo SamplerCreateInfo();
		 VkImageViewCreateInfo ImageViewCreateInfo();

		 VkRenderPassCreateInfo RenderPassCreateInfo();
		 VkFramebufferCreateInfo FramebufferCreateInfo();
		 VkRenderPassBeginInfo RenderPassBeginInfo();


		 VkViewport Viewport(float width, float height, float minDepth, float maxDepth);
		 VkRect2D Rect2D(int32_t width, int32_t height, int32_t offsetX, int32_t offsetY);

		 VkPipelineDynamicStateCreateInfo PipelineDynamicStateCreateInfo(const VkDynamicState * pDynamicStates, uint32_t dynamicStateCount, VkPipelineDynamicStateCreateFlags flags);

		 VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType type,VkShaderStageFlags stageFlags,uint32_t binding);
		 VkDescriptorSetLayoutCreateInfo DescriptorSetLayoutCreateInfo(const VkDescriptorSetLayoutBinding* pBindings,uint32_t bindingCount);
		 VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo(const VkDescriptorSetLayout* pSetLayouts,uint32_t setLayoutCount);
		 VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool descriptorPool,const VkDescriptorSetLayout* pSetLayouts,uint32_t descriptorSetCount);
		 VkDescriptorPoolCreateInfo DescriptorPoolCreateInfo(uint32_t poolSizeCount, VkDescriptorPoolSize* pPoolSizes, uint32_t maxSets);
		 VkDescriptorPoolSize DescriptorPoolSize(VkDescriptorType type, uint32_t descriptorCount);

		 VkWriteDescriptorSet  WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
		 VkWriteDescriptorSet  WriteDescriptorSet(VkDescriptorSet dstSet, VkDescriptorType type, uint32_t binding, VkDescriptorImageInfo * imageInfo);
		 VkVertexInputBindingDescription  VertexInputBindingDescription(uint32_t binding, uint32_t stride, VkVertexInputRate inputRate);
		 VkVertexInputAttributeDescription  VertexInputAttributeDescription(uint32_t binding, uint32_t location, VkFormat format, uint32_t offset);
		 VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout imageLayout);


		 VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(VkColorComponentFlags colorWriteMask, VkBool32 blendEnable);
		 VkPipelineColorBlendStateCreateInfo PipelineColorBlendStateCreateInfo(uint32_t attachmentCount, const VkPipelineColorBlendAttachmentState * pAttachments);
		 VkPipelineInputAssemblyStateCreateInfo PipelineInputAssemblyStateCreateInfo(VkPrimitiveTopology topology, VkPipelineInputAssemblyStateCreateFlags flags, VkBool32 primitiveRestartEnable);
		 VkPipelineRasterizationStateCreateInfo PipelineRasterizationStateCreateInfo(VkPolygonMode polygonMode, VkCullModeFlags cullMode, VkFrontFace frontFace, VkPipelineRasterizationStateCreateFlags flags);
		 VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo(VkBool32 depthTestEnable, VkBool32 depthWriteEnable, VkCompareOp depthCompareOp);
		 VkPipelineViewportStateCreateInfo PipelineViewportStateCreateInfo(uint32_t viewportCount, uint32_t scissorCount, VkPipelineViewportStateCreateFlags flags);
		 VkPipelineMultisampleStateCreateInfo PipelineMultisampleStateCreateInfo(VkSampleCountFlagBits rasterizationSamples, VkPipelineMultisampleStateCreateFlags flags);
		 VkGraphicsPipelineCreateInfo PipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags);
	}
}
