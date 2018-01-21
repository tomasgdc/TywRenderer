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
				vertex_shaders.resize(MAX_PIPELINES);
				fragment_shaders.resize(MAX_PIPELINES);
				geometry_shaders.resize(MAX_PIPELINES);
				compute_shaders.resize(MAX_PIPELINES);
				renderPassRef.resize(MAX_PIPELINES);
				pipelineLayoutRef.resize(MAX_PIPELINES);
				bufferLayoutRef.resize(MAX_PIPELINES);
				pipelines.resize(MAX_PIPELINES);
				input_state.resize(MAX_PIPELINES);
			}

			std::vector<DOD::Ref>								vertex_shaders;
			std::vector<DOD::Ref>								fragment_shaders;
			std::vector<DOD::Ref>								geometry_shaders;
			std::vector<DOD::Ref>								compute_shaders;
			std::vector<DOD::Ref>								renderPassRef;
			std::vector<DOD::Ref>								pipelineLayoutRef;
			std::vector<DOD::Ref>								bufferLayoutRef;
			std::vector<VkPipeline>								pipelines;
			std::vector<VkPipelineVertexInputStateCreateInfo>   input_state;
		};

		struct PipelineManager : DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<PipelineData, 
					MAX_PIPELINES>::initResourceManager();
			}

			static void		CreateResource(const DOD::Ref& ref);
			static void     DestroyResources(std::vector<DOD::Ref>& refs);

			static DOD::Ref CreatePipeline(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<PipelineData, MAX_PIPELINES>::createResource(name);

				return ref;
			}

			static void DestroyPipelineData(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<PipelineData, MAX_PIPELINES>::destroyResource(ref);
			}


			static DOD::Ref& GetVertexShader(const DOD::Ref& ref)
			{
				return data.vertex_shaders[ref._id];
			}

			static DOD::Ref& GetFragmentShader(const DOD::Ref& ref)
			{
				return data.fragment_shaders[ref._id];
			}

			static DOD::Ref& GetGeometryShader(const DOD::Ref& ref)
			{
				return data.geometry_shaders[ref._id];
			}

			static DOD::Ref& GetComputeShader(const DOD::Ref& ref)
			{
				return data.compute_shaders[ref._id];
			}

			static DOD::Ref& GetRenderPassRef(const DOD::Ref& ref)
			{
				return data.renderPassRef[ref._id];
			}

			static DOD::Ref& GetPipelineLayoutRef(const DOD::Ref& ref)
			{
				return data.pipelineLayoutRef[ref._id];
			}

			static DOD::Ref& GetbufferLayoutRef(const DOD::Ref& ref)
			{
				return data.bufferLayoutRef[ref._id];
			}

			static VkPipeline& GetPipeline(const DOD::Ref& ref)
			{
				return data.pipelines[ref._id];
			}

			static VkPipelineVertexInputStateCreateInfo& GetVertexInputState(const DOD::Ref& ref)
			{
				return data.input_state[ref._id];
			}
		};
	}
}