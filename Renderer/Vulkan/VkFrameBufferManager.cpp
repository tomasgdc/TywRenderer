#include "VkFrameBufferManager.h"
#include "VkRenderSystem.h"
#include "VkRenderPassManager.h"
#include "VkImageManager.h"
#include "VulkanTools.h"

namespace Renderer
{
	namespace Resource
	{

		void FrameBufferManager::CreateResource(const DOD::Ref& ref)
		{
			AttachementInfoArray& attachedImgs = FrameBufferManager::GetAttachedImiges(ref);
			assert(!attachedImgs.empty());

				// Collect image views from images
				std::vector<VkImageView> attachments;
				{
					attachments.resize(attachedImgs.size());
					for (uint32_t attIdx = 0u; attIdx < attachments.size(); ++attIdx)
					{
						AttachmentInfo attachmentInfo = attachedImgs[attIdx];
						attachments[attIdx] = Resource::ImageManager::GetSubresourceImageViews(attachmentInfo.imageRef, attachmentInfo.arrayLayerIdx,  0u);
					}
				}

				VkFramebufferCreateInfo fbCreateInfo = {};
				{
					fbCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
					fbCreateInfo.pNext = nullptr;

					DOD::Ref renderPassRef = GetRenderPassRef(ref);
					fbCreateInfo.renderPass = Resource::RenderPassManager::GetRenderPass(renderPassRef);

					fbCreateInfo.attachmentCount = (uint32_t)attachments.size();
					fbCreateInfo.pAttachments = attachments.data();

					const glm::uvec2& dimensions = FrameBufferManager::GetDimensions(ref);
					fbCreateInfo.width = (uint32_t)dimensions.x;
					fbCreateInfo.height = (uint32_t)dimensions.y;
					fbCreateInfo.layers = 1u;
				}

				VkFramebuffer& vkFrameBuffer = FrameBufferManager::GetFrameBuffer(ref);
				VK_CHECK_RESULT(vkCreateFramebuffer(Renderer::Vulkan::RenderSystem::vkDevice, &fbCreateInfo, nullptr, &vkFrameBuffer));
		}

		void FrameBufferManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for (uint32_t i = 0u; i < refs.size(); ++i)
			{
				const DOD::Ref& ref = refs[i];
				VkFramebuffer& framebuffer = GetFrameBuffer(ref);

				if (framebuffer != VK_NULL_HANDLE)
				{
					vkDestroyFramebuffer(Vulkan::RenderSystem::vkDevice, framebuffer, nullptr);
					framebuffer = VK_NULL_HANDLE;
				}
			}
		}
	}
}