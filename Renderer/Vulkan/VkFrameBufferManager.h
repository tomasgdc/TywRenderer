#pragma once
#include <vector>
#include <External\vulkan\vulkan.h>
#include "../DODResource.h"
#include "../../External/glm/glm/vec2.hpp"

namespace Renderer
{
	const uint32_t MAX_FRAMEBUFFER_COUNT = 1024u;

	struct AttachmentInfo
	{
		AttachmentInfo(DOD::Ref p_ImageRef, uint32_t p_ArrayLayerIdx = 0u)
			: imageRef(p_ImageRef), arrayLayerIdx(p_ArrayLayerIdx)
		{
		}

		DOD::Ref imageRef;
		uint32_t arrayLayerIdx;
	};

	typedef std::vector<AttachmentInfo> AttachementInfoArray;

	namespace Resource
	{
		struct FrameBufferData : DOD::Resource::ResourceDatabase
		{
			FrameBufferData() : ResourceDatabase(MAX_FRAMEBUFFER_COUNT)
			{
				renderPassRef.resize(MAX_FRAMEBUFFER_COUNT);
				attachedImages.resize(MAX_FRAMEBUFFER_COUNT);
				dimensions.resize(MAX_FRAMEBUFFER_COUNT);
				frameBuffers.resize(MAX_FRAMEBUFFER_COUNT);
			}

			std::vector<DOD::Ref> renderPassRef;
			std::vector<AttachementInfoArray> attachedImages;
			std::vector<glm::uvec2> dimensions;
			std::vector<VkFramebuffer> frameBuffers;
		};

		struct FrameBufferManager : DOD::Resource::ResourceManagerBase<FrameBufferData, MAX_FRAMEBUFFER_COUNT>
		{
			static void init()
			{
				DOD::Resource::ResourceManagerBase<FrameBufferData,
					MAX_FRAMEBUFFER_COUNT>::initResourceManager();
			}

			static DOD::Ref CreateFrameBuffer(const std::string& p_Name)
			{
				DOD::Ref ref = DOD::Resource::ResourceManagerBase<
					FrameBufferData, MAX_FRAMEBUFFER_COUNT>::createResource(p_Name);

				return ref;
			}

			static void DestroyFrameBufferAndResources(const std::vector<DOD::Ref>& refs);

			static void DestroyFrameBuffer(const DOD::Ref& ref)
			{
				DOD::Resource::ResourceManagerBase<
					FrameBufferData, MAX_FRAMEBUFFER_COUNT>::destroyResource(ref);
			}

			static void CreateResource(const DOD::Ref& ref);
			static void DestroyResources(const std::vector<DOD::Ref>& refs);

			static void ResetToDefault(const DOD::Ref& ref)
			{
				data.attachedImages[ref._id].clear();
				data.dimensions[ref._id] = glm::uvec2(0, 0);
			}

			static VkFramebuffer& GetFrameBuffer(const DOD::Ref& ref)
			{
				return data.frameBuffers[ref._id];
			}

			static AttachementInfoArray& GetAttachedImiges(const DOD::Ref& ref)
			{
				return data.attachedImages[ref._id];
			}

			static DOD::Ref& GetRenderPassRef(const DOD::Ref& ref)
			{
				return data.renderPassRef[ref._id];
			}

			static glm::uvec2& GetDimensions(const DOD::Ref& ref)
			{
				return data.dimensions[ref._id];
			}
		};
	}
}