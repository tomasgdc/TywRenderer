#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_DRAW_CALLS = (1024 * 10);

		struct BindingInfo
		{
			uint32_t binding_location;
			DOD::Ref buffer_ref;
		};

		struct DrawCallData : DOD::Resource::ResourceDatabase
		{
			DrawCallData() : ResourceDatabase(MAX_DRAW_CALLS)
			{
				binding_infos.resize(MAX_DRAW_CALLS);
				vertex_count.resize(MAX_DRAW_CALLS);
				index_count.resize(MAX_DRAW_CALLS);
				descriptor_sets.resize(MAX_DRAW_CALLS);
				vertex_buffer_ref.resize(MAX_DRAW_CALLS);
				index_buffer_ref.resize(MAX_DRAW_CALLS);
				pipeline_layout_references.resize(MAX_DRAW_CALLS);
				pipeline_ref.resize(MAX_DRAW_CALLS);
			}

			std::vector<std::vector<BindingInfo>> binding_infos;
			std::vector<VkDescriptorSet> descriptor_sets;

			std::vector<uint32_t>    vertex_count;
			std::vector<uint32_t>    index_count;

			std::vector<DOD::Ref>	 vertex_buffer_ref;
			std::vector<DOD::Ref>	 index_buffer_ref;

			std::vector<DOD::Ref>	 pipeline_layout_references;
			std::vector<DOD::Ref>    pipeline_ref;
				
		};

		struct DrawCallManager : DOD::Resource::ResourceManagerBase<DrawCallData, MAX_DRAW_CALLS>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<DrawCallData,
					MAX_DRAW_CALLS>::initResourceManager();
			}

			static DOD::Ref CreateDrawCall(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					DrawCallData, MAX_DRAW_CALLS>::createResource(p_Name);

				return ref;
			}

			static void CreateResource(const DOD::Ref& ref);
			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static std::vector<BindingInfo>& GetBindingInfo(const DOD::Ref& ref)
			{
				return data.binding_infos[ref._id];
			}

			static VkDescriptorSet& GetDescriptorSet(const DOD::Ref& ref)
			{
				return data.descriptor_sets[ref._id];
			}

			static DOD::Ref& GetPipelineLayoutRef(const DOD::Ref& ref)
			{
				return data.pipeline_layout_references[ref._id];
			}

			static DOD::Ref& GetPipelineRef(const DOD::Ref& ref)
			{
				return data.pipeline_ref[ref._id];
			}

			static DOD::Ref& GetVertexBufferRef(const DOD::Ref& ref)
			{
				return data.vertex_buffer_ref[ref._id];
			}

			static DOD::Ref& GetIndexBufferRef(const DOD::Ref& ref)
			{
				return data.index_buffer_ref[ref._id];
			}

			static uint32_t& GetVertexCount(const DOD::Ref& ref)
			{
				return data.vertex_count[ref._id];
			}

			static uint32_t& GetIndexCount(const DOD::Ref& ref)
			{
				return data.index_count[ref._id];
			}

		};
	}
}