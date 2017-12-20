#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"

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
			}

			std::vector<VkRenderPass> render_pass;
		};

		struct RenderPassManager : DOD::Resource::ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<RenderPassData,
					MAX_RENDER_PASS_COUNT>::initResourceManager();
			}

			static void		CreateResource(const DOD::Ref& ref);

			static DOD::Ref CreateRenderPass(const std::string& name)
			{
				DOD::Ref ref = DOD::Resource::
					ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>::createResource(name);

				return ref;
			}

			static void DestroyRenderPassData(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<RenderPassData, MAX_RENDER_PASS_COUNT>::destroyResource(ref);
			}

			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static VkRenderPass& GetRenderPass(const DOD::Ref& ref)
			{
				return data.render_pass[ref._id];
			}

		};
	}
}