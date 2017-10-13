#include "VkPipelineLayoutManager.h"

#include "VulkanTools.h"
#include "VulkanRendererInitializer.h"

namespace Renderer
{
	namespace Resource
	{
		void PipelineLayoutManager::CreatePipelineLayout(const DOD::Ref& ref)
		{
			VkPipelineLayout& pipeline_layout = PipelineLayoutManager::GetPipelineLayout(ref);
			VkDescriptorSetLayout& descriptor_set_layout = PipelineLayoutManager::GetDescriptorSetLayout(ref);
			VkDescriptorPool& descriptor_pool = PipelineLayoutManager::GetDescriptorPool(ref);

			std::vector<VkDescriptorSetLayoutBinding> set_layout_bindings;

			VkDescriptorSetLayoutCreateInfo descriptor_layout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(set_layout_bindings.data(), set_layout_bindings.size());
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(VulkanSwapChain::device, &descriptor_layout, nullptr, &descriptor_set_layout));

			VkPipelineLayoutCreateInfo pipeline_layout_create_info = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptor_set_layout, 1);
			VK_CHECK_RESULT(vkCreatePipelineLayout(VulkanSwapChain::device, &pipeline_layout_create_info, nullptr, &pipeline_layout));
		}
	}
}