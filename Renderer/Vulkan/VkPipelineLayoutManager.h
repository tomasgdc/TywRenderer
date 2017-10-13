#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"
#include "VulkanSwapChain.h"

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
				descriptor_pools.resize(MAX_PIPELINE_LAYOUT_COUNT);
			}

			std::vector<VkPipelineLayout>	   pipeline_layouts;
			std::vector<VkDescriptorSetLayout> descriptor_set_layouts;
			std::vector<VkDescriptorPool>	   descriptor_pools;
		};

		struct PipelineLayoutManager : DOD::Resource::ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<PipelineLayoutData,
					MAX_PIPELINE_LAYOUT_COUNT>::initResourceManager();
			}

			static void	CreatePipelineLayout(const DOD::Ref& ref);

			static DOD::Ref CreatePipelineLayoutData(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>::createResource(name);

				return ref;
			}

			static void DestroyPipelineLayoutData(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<PipelineLayoutData, MAX_PIPELINE_LAYOUT_COUNT>::destroyResource(ref);
			}

			static void DestroyResources(const std::vector<DOD::Ref>& refs)
			{
				for (uint32_t i = 0; i < refs.size(); i++)
				{
					DOD::Ref ref = refs[i];
					VkPipelineLayout& pipeline_layout = GetPipelineLayout(ref);

					if (pipeline_layout != VK_NULL_HANDLE)
					{
						vkDestroyPipelineLayout(VulkanSwapChain::device, pipeline_layout, nullptr);
						pipeline_layout = VK_NULL_HANDLE;
					}
				}
			}


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
		};
	}
}