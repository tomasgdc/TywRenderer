#include "Vulkan\VkPipelineManager.h"

#include "VulkanRendererInitializer.h"
#include "Vulkan\VkRenderPassManager.h"
#include "Vulkan\VkPipelineLayoutManager.h"

namespace Renderer
{
	namespace Resource
	{
		void PipelineManager::CreateGraphicsPipelineData(const DOD::Ref& ref)
		{
			VkRenderPass& render_pass = RenderPassManager::GetRenderPass(ref);
			VkPipeline&   pipeline	  = PipelineManager::GetPipeline(ref);
			VkPipelineLayout& pipeline_layout = PipelineLayoutManager::GetPipelineLayout(ref);

			VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
			inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

			VkPipelineRasterizationStateCreateInfo rasterizationState = {};
			rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
			rasterizationState.cullMode = VK_CULL_MODE_NONE;
			rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			rasterizationState.depthClampEnable = VK_FALSE;
			rasterizationState.rasterizerDiscardEnable = VK_FALSE;
			rasterizationState.depthBiasEnable = VK_FALSE;
			rasterizationState.lineWidth = 1.0f;


			VkPipelineColorBlendStateCreateInfo colorBlendState = {};
			colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
			blendAttachmentState[0].colorWriteMask = 0xf;
			blendAttachmentState[0].blendEnable = VK_FALSE;
			colorBlendState.attachmentCount = 1;
			colorBlendState.pAttachments = blendAttachmentState;

			VkPipelineViewportStateCreateInfo viewportState = {};
			viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			viewportState.viewportCount = 1;
			viewportState.scissorCount = 1;

			VkPipelineDynamicStateCreateInfo dynamicState = {};

			std::vector<VkDynamicState> dynamicStateEnables;
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
			dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);

			dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			dynamicState.pDynamicStates = dynamicStateEnables.data();
			dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());


			VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
			depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			depthStencilState.depthTestEnable = VK_TRUE;
			depthStencilState.depthWriteEnable = VK_TRUE;
			depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
			depthStencilState.depthBoundsTestEnable = VK_FALSE;
			depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
			depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
			depthStencilState.stencilTestEnable = VK_FALSE;
			depthStencilState.front = depthStencilState.back;

			VkPipelineMultisampleStateCreateInfo multisampleState = {};
			multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			multisampleState.pSampleMask = NULL;
			multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			// Load shaders
			//Shaders are loaded from the SPIR-V format, which can be generated from glsl
			//std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
			//shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/StaticModel/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
			//shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/StaticModel/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


			// Create Pipeline state VI-IA-VS-VP-RS-FS-CB
			VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

			pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCreateInfo.layout = pipeline_layout;
			pipelineCreateInfo.renderPass = render_pass;

			//pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
			//pipelineCreateInfo.pStages = shaderStages.data();
			pipelineCreateInfo.pVertexInputState = nullptr;
			pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
			pipelineCreateInfo.pRasterizationState = &rasterizationState;
			pipelineCreateInfo.pColorBlendState = &colorBlendState;
			pipelineCreateInfo.pMultisampleState = &multisampleState;
			pipelineCreateInfo.pViewportState = &viewportState;
			pipelineCreateInfo.pDepthStencilState = &depthStencilState;
			pipelineCreateInfo.pDynamicState = &dynamicState;

			// Create rendering pipeline
			VK_CHECK_RESULT(vkCreateGraphicsPipelines(VulkanSwapChain::device, VulkanRendererInitializer::m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));

		}
		
	}
}