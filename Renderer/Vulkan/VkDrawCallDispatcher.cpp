#include "VkDrawCallDispatcher.h"
#include "DrawCallManager.h"
#include "VkPipelineLayoutManager.h"
#include "VkPipelineManager.h"
#include "VkRenderPassManager.h"
#include "VkBufferObjectManager.h"
#include "VkRenderSystem.h"
#include "VulkanTools.h"

#include "VulkanRendererInitializer.h"
#include <array>

namespace Renderer
{
	namespace Vulkan
	{
		uint32_t secondaryCommandBufferIndex = 0;

		void DrawCall::BuildCommandBuffer(const DOD::Ref& ref, int width, int height)
		{
			const DOD::Ref pipeline_layout_ref  = Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(ref);
			const DOD::Ref pipeline_ref = Renderer::Resource::DrawCallManager::GetPipelineRef(ref);
			const DOD::Ref vertex_buffer_ref = Renderer::Resource::DrawCallManager::GetVertexBufferRef(ref);
			const DOD::Ref index_buffer_ref = Renderer::Resource::DrawCallManager::GetIndexBufferRef(ref);
			

			const VkPipelineLayout& pipeline_layout = Renderer::Resource::PipelineLayoutManager::GetPipelineLayout(pipeline_layout_ref);
			const VkPipeline& pipeline = Renderer::Resource::PipelineManager::GetPipeline(pipeline_ref);
			const VkRenderPass& render_pass = Renderer::Resource::RenderPassManager::GetRenderPass(ref);


			//Renderer::Vulkan::RenderSystem::BeginSecondaryComandBuffer(secondaryCommandBufferIndex, render_pass, );

			//VkCommandBuffer command_buffer;
			//VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(VulkanRendererInitializer::m_CmdPool,  VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
			//VK_CHECK_RESULT(vkAllocateCommandBuffers(Vulkan::RenderSystem::vkDevice, &cmdBufAllocateInfo, &command_buffer));

			std::array<VkClearValue, 2> clearValues;
			clearValues[0].color = { { 0.5f, 0.5f, 0.5f, 1.0f } };
			clearValues[1].depthStencil = { 1.0f, 0 };
			

			VkRenderPassBeginInfo renderPassBeginInfo = VkTools::Initializer::RenderPassBeginInfo();
			renderPassBeginInfo.renderPass = render_pass;
			
			renderPassBeginInfo.renderArea.offset.x = 0;
			renderPassBeginInfo.renderArea.offset.y = 0;
			renderPassBeginInfo.renderArea.extent.width = width;
			renderPassBeginInfo.renderArea.extent.height = height;
			renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassBeginInfo.pClearValues = clearValues.data();

			VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

			for (int i = 0; i < VulkanRendererInitializer::m_DrawCmdBuffers.size(); i++)
			{
				renderPassBeginInfo.framebuffer = VulkanRendererInitializer::m_FrameBuffers[i];
				VK_CHECK_RESULT(vkBeginCommandBuffer(VulkanRendererInitializer::m_DrawCmdBuffers[i], &cmdBufInfo));

				vkCmdBeginRenderPass(VulkanRendererInitializer::m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

				//DO SOMETHING
				{
					VkViewport viewport = VkTools::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
					vkCmdSetViewport(VulkanRendererInitializer::m_DrawCmdBuffers[i], 0, 1, &viewport);

					VkRect2D scissor = VkTools::Initializer::Rect2D(width, height, 0, 0);
					vkCmdSetScissor(VulkanRendererInitializer::m_DrawCmdBuffers[i], 0, 1, &scissor);

					vkCmdBindPipeline(VulkanRendererInitializer::m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

					// Bind triangle vertex buffer (contains position and colors)
					VkDeviceSize offsets[1] = { 0 };

					const VkDescriptorSet& descriptor_set = Renderer::Resource::DrawCallManager::GetDescriptorSet(ref);
					const VkBuffer& vertex_buffer = Renderer::Resource::BufferObjectManager::GetBufferObject(vertex_buffer_ref).buffer;
					const VkBuffer& index_buffer = Renderer::Resource::BufferObjectManager::GetBufferObject(index_buffer_ref).buffer;

					//for (int j = 0; j < listLocalBuffersVerts.size(); j++)
					{
						// Bind descriptor sets describing shader binding points
						vkCmdBindDescriptorSets(VulkanRendererInitializer::m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, NULL);

						//Bind Buffer
						vkCmdBindVertexBuffers(VulkanRendererInitializer::m_DrawCmdBuffers[i], 0, 1, &vertex_buffer, offsets);
						vkCmdBindIndexBuffer(VulkanRendererInitializer::m_DrawCmdBuffers[i], index_buffer, 0, VK_INDEX_TYPE_UINT32);

						//Draw
						const uint32_t index_count = Renderer::Resource::DrawCallManager::GetIndexCount(ref);
						vkCmdDrawIndexed(VulkanRendererInitializer::m_DrawCmdBuffers[i], index_count, 1, 0, 0, 1);
					}

				}

				vkCmdEndRenderPass(VulkanRendererInitializer::m_DrawCmdBuffers[i]);

				// Add a present memory barrier to the end of the command buffer
				// This will transform the frame buffer color attachment to a
				// new layout for presenting it to the windowing system integration 
				VkImageMemoryBarrier prePresentBarrier = {};
				prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				prePresentBarrier.pNext = NULL;
				prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
				prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
				prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
				prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
				prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
				prePresentBarrier.image = VulkanRendererInitializer::m_SwapChain.buffers[i].image;

				VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
				vkCmdPipelineBarrier(
					VulkanRendererInitializer::m_DrawCmdBuffers[i],
					VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
					VK_FLAGS_NONE,
					0, nullptr,
					0, nullptr,
					1, &prePresentBarrier);

				VK_CHECK_RESULT(vkEndCommandBuffer(VulkanRendererInitializer::m_DrawCmdBuffers[i]));
			}
		}
	}
}