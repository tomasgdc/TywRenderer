/*
*	Copyright 2015-2017 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
//stdafx
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "TywRendererHelpers.h"
#include "TywRenderer.h"

//Vulkan RendererIncludes
#include "VKRenderer.h"


//Other Includes
#include <cstdio>  //http://www.cplusplus.com/reference/cstdio/vfprintf/
#include <cstdint> //http://en.cppreference.com/w/cpp/header/cstdint
#include <cstring> //http://en.cppreference.com/w/cpp/string/byte/memcpy
#include <memory>  //smart pointer


static std::unique_ptr<IRenderer>			 g_pRenderer = nullptr;
static bool									 g_bInitialized = false;


/*
===============================================================================================
					Functions needed for VkGraphicsPipelineCreateInfo
===============================================================================================
*/
namespace vkfx
{
	void SetPipelineInputAssemblyStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t topology, uint32_t flags, uint32_t primitiveRestartEnable)
	{

	}


	void SetPipelineRasterizationStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t polygonMode, uint32_t cullMode, uint32_t frontFace, uint32_t flags)
	{

	}


	void * GetPipelineColorBlendAttachmentState(uint32_t colorWriteMask, uint32_t blendEnable)
	{
		return nullptr;
	}


	void SetPipelineColorBlendStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t attachmentCount, const void * pAttachments)
	{

	}


	void SetPipelineDepthStencilStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t depthTestEnable, uint32_t depthWriteEnable, uint32_t depthCompareOp)
	{

	}


	void SetPipelineViewportStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t viewportCount, uint32_t scissorCount, uint32_t flags)
	{

	}


	void SetPipelineMultisampleStateCreateInfo(PipelineCreateInfoHandle handle, uint32_t rasterizationSamples, uint32_t flags)
	{

	}


	void SetPipelineDynamicStateCreateInfo(PipelineCreateInfoHandle handle, const void * pDynamicStates, uint32_t dynamicStateCount, uint32_t flags)
	{

	}


	PipelineCreateInfoHandle CreatePipelineCreateInfoHandle(PipelineLayoutHandle pipelineLayoutHandle, RenderPassHandle renderPassHandle, uint32_t flags)
	{
		return VKFX_INVALID_HANDLE;
	}

	void SetShaderStages(PipelineCreateInfoHandle handle, ShaderStageHandle* shaderStage, uint32_t stagesCount)
	{

	}

	ShaderStageHandle CreateShaderStageHandle(const std::string& fileName, uint32_t stage)
	{
		return VKFX_INVALID_HANDLE;
	}
}


/*
===============================================================================================
						Functions needed for DescriptorSetup
===============================================================================================
*/
namespace vkfx
{

	DescriptorSetLayoutBindingHandle CreateDescriptorSetLayoutBinding(uint32_t type, uint32_t stageFlags, uint32_t binding)
	{
		return VKFX_INVALID_HANDLE;
	}

	DescriptorSetLayoutHandle CreateDescriptorSetLayout(DescriptorSetLayoutBindingHandle* pData, uint64_t size)
	{
		return VKFX_INVALID_HANDLE;
	}


	PipelineLayoutHandle CreatePipelineLayout(DescriptorSetLayoutHandle handle)
	{
		return VKFX_INVALID_HANDLE;
	}
}

/*
===============================================================================================
									Main commands
===============================================================================================
*/
namespace vkfx
{
	bool initialize(uint32_t height, uint32_t width, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM))
	{
		if (g_bInitialized)
		{
			return true;
		}
		g_pRenderer = std::make_unique<VKRenderer>();

		VKFX_TRACE("Initializing \n", "%s");
		g_bInitialized = g_pRenderer->VInitRenderer(height, width, isFullscreen, MainWindowProc);
		if (!g_bInitialized)
		{

		}
		return g_bInitialized;
	}


	void shutdown()
	{

	}

	void windowResize(uint32_t height, uint32_t width)
	{

	}

	void createCommandBuffer()
	{

	}


	PipelineHandle CreatePipelineHandle(PipelineCreateInfoHandle* pCreateInfos, uint32_t createInfoCount)
	{
		return VKFX_INVALID_HANDLE;
	}

	UniformDataHandle CreateUniformBuffer(void * pData, uint64_t size)
	{
		return VKFX_INVALID_HANDLE;
	}

	BufferObjectHandle CreateBufferObject(uint32_t usageFlags, uint32_t memoryPropertyFlags, uint64_t size, void * pData)
	{
		return VKFX_INVALID_HANDLE;
	}


	void UpdateUniformBuffer(UniformDataHandle handle, void * pData, uint64_t size)
	{
		//uint8_t *pData;
		//VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
		//memcpy(pData, &uboVS, sizeof(uboVS));
		//vkUnmapMemory(g_pRenderer->m_SwapChain.device, uniformDataVS.memory);
	}

	void SubmitBufferObject(BufferObjectHandle stagingBuffer, BufferObjectHandle localBuffer, uint64_t size)
	{

	}


	void submitCommandBuffer()
	{

	}

	void frame()
	{

	}


	void log(const char* _filePath, uint16_t _line, const char* _format, va_list _argList)
	{
		char temp[2048];
		char* out = temp;
		va_list argListCopy;
		va_copy(argListCopy, _argList);
		int32_t len = snprintf(out, sizeof(temp), "%s (%d): ", _filePath, _line);
		int32_t total = len + vsnprintf(out + len, sizeof(temp) - len, _format, argListCopy);
		va_end(argListCopy);
		if ((int32_t)sizeof(temp) < total)
		{
			out = (char*)alloca(total + 1);
			vkfx::memCopy(out, temp, len);
			vsnprintf(out + len, total - len, _format, _argList);
		}
		out[total] = '\0';
		vkfx::debugOutput(out);
	}

	void debugOutput(const char* _out)
	{
		fputs(_out, stdout);
		fflush(stdout);
	}

	void memCopy(void* _dst, const void* _src, size_t _numBytes)
	{
		::memcpy(_dst, _src, _numBytes);
	}
}


/*
===============================================================================================
									Data structures
===============================================================================================
*/
namespace vkfx
{
	class VPipelineCreateInfo
	{
	public:
		VPipelineCreateInfo(VkPipelineLayout layout, VkRenderPass renderPass, VkPipelineCreateFlags flags)
		{
			m_pipelineCreateInfo = VkTools::Initializer::PipelineCreateInfo(layout, renderPass, flags);
		}

		void PipelineInputAssemblyStateCreateInfo(
			VkPrimitiveTopology topology,
			VkPipelineInputAssemblyStateCreateFlags flags,
			VkBool32 primitiveRestartEnable)
		{
			m_pipelineCreateInfo.pInputAssemblyState = &(VkTools::Initializer::PipelineInputAssemblyStateCreateInfo(topology, flags, primitiveRestartEnable));
		}

		void PipelineRasterizationStateCreateInfo(
			VkPolygonMode polygonMode,
			VkCullModeFlags cullMode,
			VkFrontFace frontFace,
			VkPipelineRasterizationStateCreateFlags flags)
		{
			m_pipelineCreateInfo.pRasterizationState = &(VkTools::Initializer::PipelineRasterizationStateCreateInfo(polygonMode, cullMode, frontFace, flags));
		}

		VkPipelineColorBlendAttachmentState PipelineColorBlendAttachmentState(
			VkColorComponentFlags colorWriteMask,
			VkBool32 blendEnable)
		{
			return VkTools::Initializer::PipelineColorBlendAttachmentState(colorWriteMask, blendEnable);
		}

		void PipelineColorBlendStateCreateInfo(
			uint32_t attachmentCount,
			const VkPipelineColorBlendAttachmentState * pAttachments)
		{
			m_pipelineCreateInfo.pColorBlendState = &(VkTools::Initializer::PipelineColorBlendStateCreateInfo(attachmentCount, pAttachments));
		}

		void PipelineDepthStencilStateCreateInfo(
			VkBool32 depthTestEnable,
			VkBool32 depthWriteEnable,
			VkCompareOp depthCompareOp)
		{
			m_pipelineCreateInfo.pDepthStencilState = &(VkTools::Initializer::PipelineDepthStencilStateCreateInfo(depthTestEnable, depthWriteEnable, depthCompareOp));
		}

		void PipelineViewportStateCreateInfo(
			uint32_t viewportCount,
			uint32_t scissorCount,
			VkPipelineViewportStateCreateFlags flags)
		{
			m_pipelineCreateInfo.pViewportState = &(VkTools::Initializer::PipelineViewportStateCreateInfo(viewportCount, scissorCount, flags));
		}

		void PipelineMultisampleStateCreateInfo(
			VkSampleCountFlagBits rasterizationSamples,
			VkPipelineMultisampleStateCreateFlags flags)
		{
			m_pipelineCreateInfo.pMultisampleState = &(VkTools::Initializer::PipelineMultisampleStateCreateInfo(rasterizationSamples, flags));
		}

		void PipelineDynamicStateCreateInfo(
			const VkDynamicState * pDynamicStates,
			uint32_t dynamicStateCount,
			VkPipelineDynamicStateCreateFlags flags)
		{
			m_pipelineCreateInfo.pDynamicState = &(VkTools::Initializer::PipelineDynamicStateCreateInfo(pDynamicStates, dynamicStateCount, flags));
		}

		inline void SetStagesCount(uint32_t iShaderStages)
		{
			m_pipelineCreateInfo.stageCount = iShaderStages;
		}

		inline void SetShaderStages(VkPipelineShaderStageCreateInfo* shaderStages)
		{
			m_pipelineCreateInfo.pStages = shaderStages;
		}

		inline void SetRenderPass(VkRenderPass renderPass)
		{
			m_pipelineCreateInfo.renderPass = renderPass;
		}

		void SetVertexInputState(VkPipelineVertexInputStateCreateInfo* inputState)
		{
			m_pipelineCreateInfo.pVertexInputState = inputState;
		}

	private:
		VkGraphicsPipelineCreateInfo m_pipelineCreateInfo;
	};


	class VCommandBuffer
	{
		VkCommandBuffer m_commandBuffer;
	};

}