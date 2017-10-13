#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_PIPELINES = 1024u;

		struct PipelineData : DOD::Resource::ResourceDatabase
		{
			PipelineData(): ResourceDatabase(MAX_PIPELINES)
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
				pipelines.resize(MAX_PIPELINES);
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
			std::vector<DOD::Ref>								vertex_shaders;
			std::vector<DOD::Ref>								fragment_shaders;
			std::vector<DOD::Ref>								geometry_shaders;
			std::vector<DOD::Ref>								compute_shaders;
			std::vector<VkPipeline>								pipelines;
		};

		struct PipelineManager : DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<PipelineData, 
					MAX_PIPELINES>::initResourceManager();
			}

			static void		CreateGraphicsPipelineData(const DOD::Ref& ref);

			static DOD::Ref CreatePipelineData(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<PipelineData, MAX_PIPELINES>::createResource(name);

				return ref;
			}

			static void DestroyPipelineData(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>::destroyResource(ref);
			}

			/*
			static const DOD::Ref GetInputAssemblyState(const DOD::Ref& ref)
			{
				return input_assembly_states[ref];
			}

			static const DOD::Ref GetRasterizationState(const DOD::Ref& ref)
			{
				return rasterization_states[ref];
			}

			static const DOD::Ref GetBlendAttachementsState(const DOD::Ref& ref)
			{
				return blend_attachements_states[ref];
			}

			static const DOD::Ref GetColorBlendState(const DOD::Ref& ref)
			{
				return color_blend_states[ref];
			}
			static const DOD::Ref GetDepthStencilState(const DOD::Ref& ref)
			{
				return depth_stencil_states[ref];
			}
			
			static const DOD::Ref GetViewportState(const DOD::Ref& ref)
			{
				return viewport_states[ref];
			}

			static const DOD::Ref GetMultisampleState(const DOD::Ref& ref)
			{
				return multisample_states[ref];
			}

			static const DOD::Ref GetDynamicState(const DOD::Ref& ref)
			{
				return dynamic_states.resize(MAX_PIPELINES);
			}

			static const GetVertexInput(const DOD::Ref& ref)
			{
				return vertex_inputs[ref];
			}

			static const GetPipelineLayouyt(const DOD::Ref& ref)
			{
				return pipeline_layouts[ref];
			}

			static const GetVertexShader(const DOD::Ref& ref)
			{
				return vertex_shaders[ref];
			}

			static const GetFragmentShader(const DOD::Ref& ref)
			{
				return fragment_shaders[ref];
			}

			static const GetGeometryShader(const DOD::Ref& ref)
			{
				return geometry_shaders[ref];
			}

			static const GetComputeShader(const DOD::Ref& ref)
			{
				return compute_shaders[ref];
			}
			*/

			static VkPipeline& GetPipeline(const DOD::Ref& ref)
			{
				return data.pipelines[ref._id];
			}
		};
	}
}