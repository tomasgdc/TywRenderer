#include "DrawCallManager.h"
#include "VkPipelineLayoutManager.h"
#include "VkRenderSystem.h"
#include "VulkanTools.h"

namespace Renderer
{
	namespace Resource
	{
		void DrawCallManager::CreateResource(const std::vector<DOD::Ref>& refs)
		{
			for (const auto& ref : refs)
			{
				auto& infos = DrawCallManager::GetBindingInfo(ref);
				DOD::Ref pipeline_layout_ref = DrawCallManager::GetPipelineLayoutRef(ref);

				VkDescriptorSet& descriptorSet = DrawCallManager::GetDescriptorSet(ref);
				descriptorSet = Renderer::Resource::PipelineLayoutManager::AllocateWriteDescriptorSet(pipeline_layout_ref, infos);
			}
		}

		void DrawCallManager::CreateDrawCallForMesh(const DOD::Ref& ref)
		{

		}

		void DrawCallManager::DestroyDrawCallsAndResources(const std::vector<DOD::Ref>& refs)
		{
			DestroyResources(refs);

			for (const auto& ref : refs)
			{
				DestroyDrawCall(ref);
			}
		}

		void DrawCallManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for (int i = 0; i < refs.size(); i++)
			{
				DOD::Ref drawCallRef = refs[i];

				VkDescriptorSet& descriptorSet = GetDescriptorSet(drawCallRef);
				DOD::Ref pipelineRef = GetPipelineRef(drawCallRef);
				DOD::Ref pipelineLayoutRef = GetPipelineLayoutRef(drawCallRef);

				if (descriptorSet != VK_NULL_HANDLE)
				{
					VkDescriptorPool descriptorPool = PipelineLayoutManager::GetDescriptorPool(pipelineLayoutRef);
					VK_CHECK_RESULT(vkFreeDescriptorSets(Renderer::Vulkan::RenderSystem::vkDevice, descriptorPool, 1u, &descriptorSet));

					descriptorSet = VK_NULL_HANDLE;
				}
			}
		}
	}
}
