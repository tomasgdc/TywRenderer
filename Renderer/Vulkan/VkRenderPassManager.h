#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"
#include "VkEnums.h"

namespace Renderer
{
	namespace Resource
	{
		const uint32_t MAX_RENDER_PASS_COUNT = 1024u;

		struct RenderPassData : DOD::Resource::ResourceDatabase
		{
			RenderPassData() : ResourceDatabase(MAX_RENDER_PASS_COUNT)
			{
				render_pass.resize(MAX_RENDER_PASS_COUNT);
				descAttachments.resize(MAX_RENDER_PASS_COUNT);
			}

			std::vector<VkRenderPass> render_pass;
			std::vector<std::vector<AttachementDescription>> descAttachments;
		};

		struct RenderPassManager : DOD::Resource::ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<RenderPassData,
					MAX_RENDER_PASS_COUNT>::initResourceManager();
			}

			static DOD::Ref GetResourceByName(const std::string& name)
			{
				auto resourceIt = nameResourceMap.find(name);
				if (resourceIt == nameResourceMap.end())
				{
					return DOD::Ref();
				}
				return resourceIt->second;
			}

			static void CreateAllResources()
			{
				DestroyResources(activeRefs);
				CreateResource(activeRefs);
			}

			static void CreateResource(const std::vector<DOD::Ref>& ref);

			static void ResetToDefault(const DOD::Ref& ref)
			{
				data.descAttachments[ref._id].clear();
			}

			static DOD::Ref CreateRenderPass(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>::createResource(name);

				return ref;
			}

			static void DestroyRenderPassAndResources(const std::vector<DOD::Ref>& refs)
			{
				DestroyResources(refs);

				for (const auto& ref : refs)
				{
					DestroyRenderPass(ref);
				}
			}

			static void DestroyRenderPass(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>::destroyResource(ref);
			}

			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static VkRenderPass& GetRenderPass(const DOD::Ref& ref)
			{
				return data.render_pass[ref._id];
			}

			static std::vector<AttachementDescription>& GetAttachementDescription(const DOD::Ref& ref)
			{
				return data.descAttachments[ref._id];
			}

		};
	}
}