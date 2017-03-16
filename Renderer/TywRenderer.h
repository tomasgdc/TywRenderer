/*
*	Copyright 2015-2017 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once


#define VKFX_HANDLE(_name) \
			struct _name { uint16_t idx; }; \
			inline bool isValid(_name _handle) { return vkfx::invalidHandle != _handle.idx; }

#define VKFX_INVALID_HANDLE  { vkfx::invalidHandle }

/*
===============================================================================================
					Program handles that are used in this library
===============================================================================================
*/
namespace vkfx
{
	static const uint16_t invalidHandle = UINT16_MAX;

	/*
	Used for framebuffer
	*/
	VKFX_HANDLE(RenderPassHandle);
	VKFX_HANDLE(FrameBufferHandle);
	VKFX_HANDLE(FrameBufferAttachmentHandle);
	VKFX_HANDLE(ColorSamplerHandle);

	/*
	Used for pipeline, descriptor and command buffer
	*/
	VKFX_HANDLE(PipelineCreateInfoHandle);
	VKFX_HANDLE(CommandBufferHandle);
	VKFX_HANDLE(PipelineHandle)
	VKFX_HANDLE(PipelineLayoutHandle);
	VKFX_HANDLE(DescriptorSetHandle);
	VKFX_HANDLE(DescriptorSetLayoutHandle);
	VKFX_HANDLE(ShaderStageHandle);

	/*
	Used for storing data on gpu
	*/
	VKFX_HANDLE(UniformDataHandle);
	VKFX_HANDLE(BufferObjectHandle);
}


/*
===============================================================================================
					Functions needed for VkGraphicsPipelineCreateInfo
===============================================================================================
*/
namespace vkfx
{
	/*
		@param: PipelineCreateInfoHandle handle
		@param: VkPrimitiveTopology topology
		@param: VkPipelineInputAssemblyStateCreateFlags flags
		@param: VkBool32 primitiveRestartEnable
	*/
	void SetPipelineInputAssemblyStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t topology, uint32_t flags, uint32_t primitiveRestartEnable);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: VkPolygonMode polygonMode,
		@param: VkCullModeFlags cullMode,
		@param: VkFrontFace frontFace,
		@param: VkPipelineRasterizationStateCreateFlags flags
	*/
	void SetPipelineRasterizationStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t polygonMode, uint32_t cullMode, uint32_t frontFace, uint32_t flags);


	/*
		@param:  VkColorComponentFlags colorWriteMask,
		@param:  VkBool32 blendEnable
		@return: VkPipelineColorBlendAttachmentState
	*/
	void * GetPipelineColorBlendAttachmentState(uint32_t colorWriteMask, uint32_t blendEnable);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: uint32_t attachmentCount,
		@param: const VkPipelineColorBlendAttachmentState * pAttachments
	*/
	void SetPipelineColorBlendStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t attachmentCount, const void * pAttachments);

	/*
		@param: PipelineCreateInfoHandle handle
		@param:	VkBool32 depthTestEnable,
		@param: VkBool32 depthWriteEnable,
		@param: VkCompareOp depthCompareOp
	*/
	void SetPipelineDepthStencilStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t depthTestEnable, uint32_t depthWriteEnable, uint32_t depthCompareOp);

	/*
		@param: int32_t viewportCount,
		@param: int32_t scissorCount,
		@param: VkPipelineViewportStateCreateFlags flags
	*/
	void SetPipelineViewportStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t viewportCount, uint32_t scissorCount, uint32_t flags);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: VkSampleCountFlagBits rasterizationSamples,
		@param:	VkPipelineMultisampleStateCreateFlags flags
	*/
	void SetPipelineMultisampleStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t rasterizationSamples, uint32_t flags);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: const VkDynamicState * pDynamicStates,
		@param: uint32_t dynamicStateCount,
		@param: VkPipelineDynamicStateCreateFlags flags
	*/
	void SetPipelineDynamicStateCreateInfo(PipelineCreateInfoHandle handle, const void * pDynamicStates, uint32_t dynamicStateCount, uint32_t flags);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: VkPipelineLayout layout,
		@param: VkRenderPass renderPass,
		@parma: VkPipelineCreateFlags flags
	*/
	PipelineCreateInfoHandle CreatePipelineCreateInfoHandle(PipelineLayoutHandle pipelineLayoutHandle, RenderPassHandle renderPassHandle, uint32_t flags);

	/*
		@param: PipelineCreateInfoHandle handle
		@param: ShaderStageHandle shaderStage
		@param: uint32_t stagesCount
	*/
	void SetShaderStages(PipelineCreateInfoHandle handle, ShaderStageHandle* shaderStage, uint32_t stagesCount);

	/*
		@param: std::string fileName
		@param: VkShaderStageFlagBits
		@return: ShaderStageHandle
	*/
	ShaderStageHandle CreateShaderStageHandle(const std::string& fileName, uint32_t stage);
}


/*
===============================================================================================
						Main commands
===============================================================================================
*/
namespace vkfx
{
	/*
		Initialize
	*/
	bool initialize(uint32_t height, uint32_t width, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM));

	/*
	*/
	void shutdown();

	/*
	*/
	void windowResize(uint32_t height, uint32_t width);

	/*

	*/
	void createCommandBuffer();

	/*
		Same as vkCreateGraphicsPipelines
		TODO: Add Allocation Callbacks ?

		@param: PipelineCreateInfoHandle* pCreateInfos
		@param: uint32_t createInfoCount
		@return PipelineHandle
	*/
	PipelineHandle CreatePipelineHandle(PipelineCreateInfoHandle* pCreateInfos, uint32_t createInfoCount);


	/*
		@param:  void * pData - pointer to actuall data
		@param:  uint64_t size
		@return: UniformDataHandle
	*/
	UniformDataHandle CreateUniformBuffer(void * pData, uint64_t size);

	/*
	*/
	void submitCommandBuffer();


	/*
	
	*/
	

	/*
		Render
	*/
	void frame();
}