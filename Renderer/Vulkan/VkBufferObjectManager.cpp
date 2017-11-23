#include "VkBufferObjectManager.h"
#include "VulkanTools.h"
#include "VulkanSwapChain.h"

#include "VulkanRendererInitializer.h"

namespace Renderer
{
	namespace Resource
	{
		void BufferObjectManager::CreateResource(const DOD::Ref& ref, VkPhysicalDeviceMemoryProperties&  memory_properties)
		{
			BufferObject& buffer_object = BufferObjectManager::GetBufferObject(ref);
			const VkDeviceSize data_size = Renderer::Resource::BufferObjectManager::GetBufferSize(ref);
			const VkBufferUsageFlags usage_flags = Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(ref) | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			void*& data = Renderer::Resource::BufferObjectManager::GetBufferData(ref);

			// Create the buffer handle
			VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo(usage_flags, data_size);
			VK_CHECK_RESULT(vkCreateBuffer(VulkanSwapChain::device, &bufferCreateInfo, nullptr, &buffer_object.buffer));

			// Create the memory backing up the buffer handle
			VkMemoryRequirements memReqs;
			VkMemoryAllocateInfo memAlloc = VkTools::Initializer::MemoryAllocateInfo();

			vkGetBufferMemoryRequirements(VulkanSwapChain::device, buffer_object.buffer, &memReqs);
			memAlloc.allocationSize = memReqs.size;

			// Find a memory type index that fits the properties of the buffer
			memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, memory_properties);
			VK_CHECK_RESULT(vkAllocateMemory(VulkanSwapChain::device, &memAlloc, nullptr, &buffer_object.memory));


			// Attach the memory to the buffer object
			VK_CHECK_RESULT(vkBindBufferMemory(VulkanSwapChain::device, buffer_object.buffer, buffer_object.memory, 0));

			BufferObject temp_staging_buffer;
			VkBufferCreateInfo stagingBufferCreateInfo = bufferCreateInfo;
			{
				stagingBufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			}

			VK_CHECK_RESULT(vkCreateBuffer(VulkanSwapChain::device, &stagingBufferCreateInfo, nullptr, &temp_staging_buffer.buffer));

			VkMemoryRequirements stagingMemReqs;
			vkGetBufferMemoryRequirements(VulkanSwapChain::device, temp_staging_buffer.buffer, &stagingMemReqs);

			memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, memory_properties);
			VK_CHECK_RESULT(vkAllocateMemory(VulkanSwapChain::device, &memAlloc, nullptr, &temp_staging_buffer.memory));
			VK_CHECK_RESULT(vkBindBufferMemory(VulkanSwapChain::device, temp_staging_buffer.buffer, temp_staging_buffer.memory, 0));

			// If a pointer to the buffer data has been passed, map the buffer and copy over the data
			if (data != nullptr)
			{
				void *mapped;
				VK_CHECK_RESULT(vkMapMemory(VulkanSwapChain::device, temp_staging_buffer.memory, 0, data_size, 0, &mapped));
				memcpy(mapped, data, data_size);
				vkUnmapMemory(VulkanSwapChain::device, temp_staging_buffer.memory);
			}


			// Buffer copies have to be submitted to a queue, so we need a command buffer for them
			// Note that some devices offer a dedicated transfer queue (with only the transfer bit set)
			// If you do lots of copies (especially at runtime) it's advised to use such a queu instead
			// of a generalized graphics queue (that also supports transfers)
			VkCommandBuffer cmd_buffer;

			VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
			cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufAllocateInfo.commandPool = VulkanRendererInitializer::m_CmdPool;
			cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufAllocateInfo.commandBufferCount = 1;

			VK_CHECK_RESULT(vkAllocateCommandBuffers(VulkanSwapChain::device, &cmdBufAllocateInfo, &cmd_buffer));
			VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
			VK_CHECK_RESULT(vkBeginCommandBuffer(cmd_buffer, &cmdBufInfo));

			// Put buffer region copies into command buffer
			// Note that the staging buffer must not be deleted before the copies have been submitted and executed
			VkBufferCopy copyRegion = {0,0, data_size};
			vkCmdCopyBuffer(cmd_buffer, temp_staging_buffer.buffer, buffer_object.buffer, 1, &copyRegion);

			//Flush Command Buffer
			VK_CHECK_RESULT(vkEndCommandBuffer(cmd_buffer));

			VkSubmitInfo submitInfo = {};
			submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &cmd_buffer;

			VK_CHECK_RESULT(vkQueueSubmit(VulkanRendererInitializer::m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
			VK_CHECK_RESULT(vkQueueWaitIdle(VulkanRendererInitializer::m_Queue));

			vkFreeCommandBuffers(VulkanSwapChain::device, VulkanRendererInitializer::m_CmdPool, 1, &cmd_buffer);

			//Delete stagging buffer data as all needed data is sent to gpu
			vkDestroyBuffer(VulkanSwapChain::device, temp_staging_buffer.buffer, nullptr);
			vkFreeMemory(VulkanSwapChain::device, temp_staging_buffer.memory, nullptr);
		}

		void BufferObjectManager::DestroyResources(const std::vector<DOD::Ref>& refs)
		{
			for (auto& ref : refs)
			{
				BufferObject& buffer_object = BufferObjectManager::GetBufferObject(ref);

				if (buffer_object.buffer != VK_NULL_HANDLE)
				{
					vkDestroyBuffer(VulkanSwapChain::device, buffer_object.buffer, nullptr);
					vkFreeMemory(VulkanSwapChain::device, buffer_object.memory, nullptr);
					buffer_object.buffer = VK_NULL_HANDLE;
				}
			}
		}
	}
}