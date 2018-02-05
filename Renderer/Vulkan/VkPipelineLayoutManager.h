#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"
#include "VulkanSwapChain.h"

//BindingInfo
#include "DrawCallManager.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_PIPELINE_LAYOUT_COUNT = 1024u;

		struct PipelineLayoutData : DOD::Resource::ResourceDatabase
		{
			PipelineLayoutData() : ResourceDatabase(MAX_PIPELINE_LAYOUT_COUNT)
			{
				pipeline_layouts.resize(MAX_PIPELINE_LAYOUT_COUNT);
				descriptor_set_layouts.resize(MAX_PIPELINE_LAYOUT_COUNT);
				descriptor_set_layout_bindings.resize(MAX_PIPELINE_LAYOUT_COUNT);
				descriptor_pools.resize(MAX_PIPELINE_LAYOUT_COUNT);
			}

			std::vector<VkPipelineLayout>			   pipeline_layouts;
			std::vector<VkDescriptorSetLayout>		   descriptor_set_layouts;
			std::vector<std::vector<VkDescriptorSetLayoutBinding>>  descriptor_set_layout_bindings;
			std::vector<VkDescriptorPool>	           descriptor_pools;
			
		};

		struct PipelineLayoutManager : DOD::Resource::ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<PipelineLayoutData,
					MAX_PIPELINE_LAYOUT_COUNT>::initResourceManager();
			}

			static void CreateAllResources()
			{
				DestroyResources(activeRefs);
				CreateResource(activeRefs);
			}

			static void	CreateResource(const std::vector<DOD::Ref>& refs);

			static VkDescriptorSet AllocateWriteDescriptorSet(const DOD::Ref& ref, const std::vector<BindingInfo>& binding_infos);


			static void DestroyPipelineLayoutAndResources(const std::vector<DOD::Ref>& refs);

			static DOD::Ref CreatePipelineLayout(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>::createResource(name);

				return ref;
			}

			static void DestroyPipelineLayout(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>::destroyResource(ref);
			}

			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static VkPipelineLayout& GetPipelineLayout(const DOD::Ref& ref)
			{
				return data.pipeline_layouts[ref._id];
			}

			static VkDescriptorSetLayout& GetDescriptorSetLayout(const DOD::Ref& ref)
			{
				return data.descriptor_set_layouts[ref._id];
			}

			static VkDescriptorPool& GetDescriptorPool(const DOD::Ref& ref)
			{
				return data.descriptor_pools[ref._id];
			}

			static std::vector<VkDescriptorSetLayoutBinding>& GetDescriptorSetLayoutBinding(const DOD::Ref& ref)
			{
				return data.descriptor_set_layout_bindings[ref._id];
			}

		};
	}
}