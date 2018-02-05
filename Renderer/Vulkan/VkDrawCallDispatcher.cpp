#include "VkDrawCallDispatcher.h"
#include "DrawCallManager.h"
#include "VkPipelineLayoutManager.h"
#include "VkPipelineManager.h"
#include "VkRenderPassManager.h"
#include "VkBufferObjectManager.h"
#include "VkRenderSystem.h"
#include "VulkanTools.h"
#include "VkFrameBufferManager.h"

#include "VulkanRendererInitializer.h"
#include <array>

namespace Renderer
{
	namespace Vulkan
	{
		

		struct DrawCallParallelTask
		{
			void Execute()
			{
				const DOD::Ref pipeline_layout_ref = Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(drawCallRef);
				const DOD::Ref pipeline_ref = Renderer::Resource::DrawCallManager::GetPipelineRef(drawCallRef);
				const DOD::Ref vertex_buffer_ref = Renderer::Resource::DrawCallManager::GetVertexBufferRef(drawCallRef);
				const DOD::Ref index_buffer_ref = Renderer::Resource::DrawCallManager::GetIndexBufferRef(drawCallRef);

				const VkPipelineLayout& pipeline_layout = Renderer::Resource::PipelineLayoutManager::GetPipelineLayout(pipeline_layout_ref);
				const VkPipeline& pipeline = Renderer::Resource::PipelineManager::GetPipeline(pipeline_ref);
				const VkRenderPass& render_pass = Renderer::Resource::RenderPassManager::GetRenderPass(renderPassRef);
				const VkFramebuffer& frame_buffer = Renderer::Resource::FrameBufferManager::GetFrameBuffer(frameBufferRef);

				Renderer::Vulkan::RenderSystem::BeginSecondaryComandBuffer(secondaryCommandBufferIndex, render_pass, frame_buffer);
				VkCommandBuffer& secondaryCommandBuffer = Renderer::Vulkan::RenderSystem::GetSecondaryCommandBuffer(secondaryCommandBufferIndex);
				
				//for
				{
					//DO SOMETHING
					{
						VkViewport viewport = VkTools::Initializer::Viewport((float)width, (float)height, 0.0f, 1.0f);
						vkCmdSetViewport(secondaryCommandBuffer, 0, 1, &viewport);

						VkRect2D scissor = VkTools::Initializer::Rect2D(width, height, 0, 0);
						vkCmdSetScissor(secondaryCommandBuffer, 0, 1, &scissor);

						vkCmdBindPipeline(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

						// Bind triangle vertex buffer (contains position and colors)
						VkDeviceSize offsets[1] = { 0 };

						const VkDescriptorSet& descriptor_set = Renderer::Resource::DrawCallManager::GetDescriptorSet(drawCallRef);
						const VkBuffer& vertex_buffer = Renderer::Resource::BufferObjectManager::GetBufferObject(vertex_buffer_ref).buffer;
						const VkBuffer& index_buffer = Renderer::Resource::BufferObjectManager::GetBufferObject(index_buffer_ref).buffer;

						//for (int j = 0; j < listLocalBuffersVerts.size(); j++)
						{
							// Bind descriptor sets describing shader binding points
							vkCmdBindDescriptorSets(secondaryCommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, NULL);

							//Bind Buffer
							vkCmdBindVertexBuffers(secondaryCommandBuffer, 0, 1, &vertex_buffer, offsets);
							vkCmdBindIndexBuffer(secondaryCommandBuffer, index_buffer, 0, VK_INDEX_TYPE_UINT32);

							//Draw
							const uint32_t index_count = Renderer::Resource::DrawCallManager::GetIndexCount(drawCallRef);
							vkCmdDrawIndexed(secondaryCommandBuffer, index_count, 1, 0, 0, 1);
						}
					}
				}

				Renderer::Vulkan::RenderSystem::EndSecondaryComandBuffer(secondaryCommandBufferIndex);
			}

			DOD::Ref drawCallRef;
			DOD::Ref frameBufferRef;
			DOD::Ref renderPassRef;
			uint32_t secondaryCommandBufferIndex;
			uint32_t width;
			uint32_t height;
		};

		uint32_t secondaryCommandBufferIndex = 0;
		void DrawCall::QueuDrawCall(const DOD::Ref& ref, const DOD::Ref& frameBuffer, const DOD::Ref& renderPass, int width, int height)
		{
			DrawCallParallelTask task;
			task.drawCallRef = ref;
			task.frameBufferRef = frameBuffer;
			task.renderPassRef = renderPass;
			task.secondaryCommandBufferIndex = secondaryCommandBufferIndex;
			task.width = width;
			task.height = height;

			task.Execute();

			vkCmdExecuteCommands(RenderSystem::GetPrimaryCommandBuffer(), 1u,
				&Renderer::Vulkan::RenderSystem::GetSecondaryCommandBuffer(secondaryCommandBufferIndex));
		}
	}
}