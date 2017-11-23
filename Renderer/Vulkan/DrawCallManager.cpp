#include "DrawCallManager.h"
#include "VkPipelineLayoutManager.h"

namespace Renderer
{
	namespace Resource
	{
		void DrawCallManager::CreateResource(const DOD::Ref& ref)
		{
			auto& infos = DrawCallManager::GetBindingInfo(ref);
			DOD::Ref pipeline_layout_ref  = DrawCallManager::GetPipelineLayoutRef(ref);


			PipelineLayoutManager::AllocateWriteDescriptorSet(pipeline_layout_ref, infos);
		}

		void DrawCallManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{

		}
	}
}
