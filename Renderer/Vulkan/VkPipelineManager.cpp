#include "Vulkan\VkPipelineManager.h"

#include "VulkanRendererInitializer.h"
#include "Vulkan\VkRenderPassManager.h"
#include "Vulkan\VkPipelineLayoutManager.h"
#include "Vulkan\VkGpuProgram.h"
#include "Vulkan\VkBufferLayoutManager.h"
#include "VkRenderSystem.h"

namespace Renderer
{
    namespace Resource
    {
        void PipelineManager::CreateResource(const std::vector<DOD::Ref>& refs)
        {
			for (const auto& ref : refs)
			{
				VkPipeline&   pipeline = PipelineManager::GetPipeline(ref);

				const DOD::Ref renderPassRef = PipelineManager::GetRenderPassRef(ref);
				VkRenderPass& render_pass = RenderPassManager::GetRenderPass(renderPassRef);

				const DOD::Ref pipelineLayoutRef = PipelineManager::GetPipelineLayoutRef(ref);
				VkPipelineLayout& pipeline_layout = PipelineLayoutManager::GetPipelineLayout(pipelineLayoutRef);

				const DOD::Ref bufferLayoutRef = PipelineManager::GetbufferLayoutRef(ref);
				VkPipelineVertexInputStateCreateInfo& vertex_input = BufferLayoutManager::GetVertexInput(bufferLayoutRef);

				uint32_t shader_stage_count = 0u;
				VkPipelineShaderStageCreateInfo shader_stages[3];

				DOD::Ref vertex_shader = PipelineManager::GetVertexShader(ref);
				DOD::Ref geometry_shader = PipelineManager::GetGeometryShader(ref);
				DOD::Ref fragment_shader = PipelineManager::GetFragmentShader(ref);

				if (vertex_shader.isValid())
					shader_stages[shader_stage_count++] = GpuProgramManager::GetShaderStageCreateInfo(vertex_shader);

				if (geometry_shader.isValid())
					shader_stages[shader_stage_count++] = GpuProgramManager::GetShaderStageCreateInfo(geometry_shader);

				if (fragment_shader.isValid())
					shader_stages[shader_stage_count++] = GpuProgramManager::GetShaderStageCreateInfo(fragment_shader);

				VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
					VkTools::Initializer::PipelineInputAssemblyStateCreateInfo(
						VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
						0,
						VK_FALSE);

				VkPipelineRasterizationStateCreateInfo rasterizationState =
					VkTools::Initializer::PipelineRasterizationStateCreateInfo(
						VK_POLYGON_MODE_FILL,
						VK_CULL_MODE_BACK_BIT,
						VK_FRONT_FACE_CLOCKWISE,
						0);

				VkPipelineColorBlendAttachmentState blendAttachmentState =
					VkTools::Initializer::PipelineColorBlendAttachmentState(
						0xf,
						VK_FALSE);

				VkPipelineColorBlendStateCreateInfo colorBlendState =
					VkTools::Initializer::PipelineColorBlendStateCreateInfo(
						1,
						&blendAttachmentState);

				VkPipelineDepthStencilStateCreateInfo depthStencilState =
					VkTools::Initializer::PipelineDepthStencilStateCreateInfo(
						VK_TRUE,
						VK_TRUE,
						VK_COMPARE_OP_LESS_OR_EQUAL);

				VkPipelineViewportStateCreateInfo viewportState =
					VkTools::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);

				VkPipelineMultisampleStateCreateInfo multisampleState =
					VkTools::Initializer::PipelineMultisampleStateCreateInfo(
						VK_SAMPLE_COUNT_1_BIT,
						0);

				std::vector<VkDynamicState> dynamicStateEnables = {
					VK_DYNAMIC_STATE_VIEWPORT,
					VK_DYNAMIC_STATE_SCISSOR
				};
				VkPipelineDynamicStateCreateInfo dynamicState =
					VkTools::Initializer::PipelineDynamicStateCreateInfo(
						dynamicStateEnables.data(),
						static_cast<uint32_t>(dynamicStateEnables.size()),
						0);

				// Create Pipeline state VI-IA-VS-VP-RS-FS-CB
				VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

				pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
				pipelineCreateInfo.layout = pipeline_layout;
				pipelineCreateInfo.renderPass = render_pass;

				pipelineCreateInfo.stageCount = shader_stage_count;
				pipelineCreateInfo.pStages = shader_stages;
				pipelineCreateInfo.pVertexInputState = &vertex_input;
				pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
				pipelineCreateInfo.pRasterizationState = &rasterizationState;
				pipelineCreateInfo.pColorBlendState = &colorBlendState;
				pipelineCreateInfo.pMultisampleState = &multisampleState;
				pipelineCreateInfo.pViewportState = &viewportState;
				pipelineCreateInfo.pDepthStencilState = &depthStencilState;
				pipelineCreateInfo.pDynamicState = &dynamicState;

				// Create rendering pipeline
				VK_CHECK_RESULT(vkCreateGraphicsPipelines(Vulkan::RenderSystem::vkDevice, VulkanRendererInitializer::m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));
			}
        }

		void PipelineManager::DestroyPipelineAndResources(const std::vector<DOD::Ref>& refs)
		{
			DestroyResources(refs);

			for (const auto& ref : refs)
			{
				DestroyPipeline(ref);
			}
		}

		void PipelineManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for(const auto& ref: refs)
			{
				VkPipeline& pipeline = GetPipeline(ref);
				if (pipeline != VK_NULL_HANDLE)
				{
					vkDestroyPipeline(Vulkan::RenderSystem::vkDevice, pipeline, nullptr);
					pipeline = VK_NULL_HANDLE;
				}
			}
		}
        
    }
}