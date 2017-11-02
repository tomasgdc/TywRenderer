#include "VkPipelineLayoutManager.h"

#include "VulkanTools.h"
#include "VulkanRendererInitializer.h"

namespace Renderer
{
	namespace Resource
	{
		void PipelineLayoutManager::CreateResource(const DOD::Ref& ref)
		{
			VkPipelineLayout& pipeline_layout = PipelineLayoutManager::GetPipelineLayout(ref);
			VkDescriptorSetLayout& descriptor_set_layout = PipelineLayoutManager::GetDescriptorSetLayout(ref);
			VkDescriptorPool& descriptor_pool = PipelineLayoutManager::GetDescriptorPool(ref);

			std::vector<VkDescriptorSetLayoutBinding>& set_layout_bindings = PipelineLayoutManager::GetDescriptorSetLayoutBinding(ref);

			VkDescriptorSetLayoutCreateInfo descriptor_layout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(set_layout_bindings.data(), set_layout_bindings.size());
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanSwapChain::device, &descriptor_layout, nullptr, &descriptor_set_layout));

			VkPipelineLayoutCreateInfo pipeline_layout_create_info = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptor_set_layout, 1);
			VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanSwapChain::device, &pipeline_layout_create_info, nullptr, &pipeline_layout));


			std::vector<VkDescriptorPoolSize> pool_size;
			pool_size.reserve(set_layout_bindings.size());

			for (auto& layout_binding : set_layout_bindings)
			{
				pool_size.push_back(VkTools::Initializer::DescriptorPoolSize(layout_binding.descriptorType, layout_binding.descriptorCount));
			}

			VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(pool_size.size(), pool_size.data(), 2);
			VK_CHECK_RESULT(vkCreateDescriptorPool(VulkanSwapChain::device, &descriptorPoolInfo, nullptr, &descriptor_pool));
		}

		void PipelineLayoutManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for (uint32_t i = 0; i < refs.size(); i++)
			{
				const DOD::Ref& ref = refs[i];

				VkPipelineLayout& pipeline_layout = GetPipelineLayout(ref);
				if (pipeline_layout != VK_NULL_HANDLE)
				{
					vkDestroyPipelineLayout(VulkanSwapChain::device, pipeline_layout, nullptr);
					pipeline_layout = VK_NULL_HANDLE;
				}

				VkDescriptorSetLayout& descriptor_layout = GetDescriptorSetLayout(ref);
				if (descriptor_layout != VK_NULL_HANDLE)
				{
					vkDestroyDescriptorSetLayout(VulkanSwapChain::device, descriptor_layout, nullptr);
					descriptor_layout = VK_NULL_HANDLE;
				}

				VkDescriptorPool& descriptor_pool = GetDescriptorPool(ref);
				if (descriptor_pool != VK_NULL_HANDLE)
				{
					vkDestroyDescriptorPool(VulkanSwapChain::device, descriptor_pool, nullptr);
					descriptor_pool = VK_NULL_HANDLE;
				}
			}
		}
	}
}