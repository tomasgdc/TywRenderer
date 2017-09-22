#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "DODResource.h"

namespace Renderer
{
	namespace Pipeline
	{
		const uint32_t MAX_PIPELINES = 1024u;

		struct PipelineData : DOD::Resource::ResourceDatabase
		{
			PipelineData(uint32_t MAX_PIPELINES)
			{
				input_assembly_states.resize(MAX_PIPELINES);
				rasterization_states.resize(MAX_PIPELINES);
				blend_attachements_states.resize(MAX_PIPELINES);
				color_blend_states.resize(MAX_PIPELINES);
				depth_stencil_states.resize(MAX_PIPELINES);
				viewport_states.resize(MAX_PIPELINES);
				multisample_states.resize(MAX_PIPELINES);
				dynamic_states.resize(MAX_PIPELINES);
				vertex_inputs.resize(MAX_PIPELINES);
				pipeline_layouts.resize(MAX_PIPELINES);
				vertex_shaders.resize(MAX_PIPELINES);
				fragment_shaders.resize(MAX_PIPELINES);
				geometry_shaders.resize(MAX_PIPELINES);
				compute_shaders.resize(MAX_PIPELINES);
			}

			std::vector<VkPipelineInputAssemblyStateCreateInfo> input_assembly_states;
			std::vector<VkPipelineRasterizationStateCreateInfo> rasterization_states;
			std::vector<VkPipelineColorBlendAttachmentState>    blend_attachements_states;
			std::vector<VkPipelineColorBlendStateCreateInfo>    color_blend_states;
			std::vector<VkPipelineDepthStencilStateCreateInfo>  depth_stencil_states;
			std::vector<VkPipelineViewportStateCreateInfo>      viewport_states;
			std::vector<VkPipelineMultisampleStateCreateInfo>   multisample_states;
			std::vector<VkPipelineDynamicStateCreateInfo>       dynamic_states;
			std::vector<VkGraphicsPipelineCreateInfo>           vertex_inputs;
			std::vector<VkPipelineLayout>                       pipeline_layouts;
			std::vector<VkPipelineShaderStageCreateInfo>        vertex_shaders;
			std::vector<VkPipelineShaderStageCreateInfo>        fragment_shaders;
			std::vector<VkPipelineShaderStageCreateInfo>        geometry_shaders;
			std::vector<VkPipelineShaderStageCreateInfo>        compute_shaders;
		};

		struct PipelineManager : DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<PipelineData, 
					MAX_PIPELINES>::initResourceManager();
			}

			static void		createGraphicsPipelineData(const DOD::Ref& ref);

			static DOD::Ref createPipelineData(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<PipelineData, MAX_PIPELINES>::createResource(name);

				return ref;
			}

			static void destroyPipelineData(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>::destroyResource(ref);
			}
		};
	}
}