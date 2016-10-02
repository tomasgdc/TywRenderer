/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>


#include "VkBufferObject.h"
#include "VulkanTools.h"
#include "VulkanSwapChain.h"
#include "Geometry\VertData.h"
#include "VulkanRendererInitializer.h"
#include "VKRenderer.h"



VkBufferObject::VkBufferObject()
{

}

VkResult VkBufferObject::CreateBuffer(const VulkanSwapChain& pSwapChain, VkPhysicalDeviceMemoryProperties& memoryProperties, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBufferObject_s& bufferObject, void *data)
{
	// Create the buffer handle
	VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo(usageFlags, size);
	VK_CHECK_RESULT(vkCreateBuffer(pSwapChain.device, &bufferCreateInfo, nullptr, &bufferObject.buffer));

	// Create the memory backing up the buffer handle
	VkMemoryRequirements memReqs;
	VkMemoryAllocateInfo memAlloc = VkTools::Initializer::MemoryAllocateInfo();
	vkGetBufferMemoryRequirements(pSwapChain.device, bufferObject.buffer, &memReqs);
	memAlloc.allocationSize = memReqs.size;

	// Find a memory type index that fits the properties of the buffer
	memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags, memoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(pSwapChain.device, &memAlloc, nullptr, &bufferObject.memory));

	// If a pointer to the buffer data has been passed, map the buffer and copy over the data
	if (data != nullptr)
	{
		void *mapped;
		VK_CHECK_RESULT(vkMapMemory(pSwapChain.device, bufferObject.memory, 0, size, 0, &mapped));
		memcpy(mapped, data, size);
		vkUnmapMemory(pSwapChain.device, bufferObject.memory);
	}

	// Attach the memory to the buffer object
	VK_CHECK_RESULT(vkBindBufferMemory(pSwapChain.device, bufferObject.buffer, bufferObject.memory, 0));
	return VK_SUCCESS;
}


VkResult VkBufferObject::SubmitBufferObjects(const VkCommandBuffer& copyCmd, const VkQueue& copyQueue, const VulkanRendererInitializer& pRendInit, VkDeviceSize size, VkBufferObject_s& stagingBuffer, VkBufferObject_s& localBuffer, drawVertFlags enumDrawDescriptors)
{
	VkBufferCopy copyRegion = {};

	copyRegion.size = size;
	vkCmdCopyBuffer(
		copyCmd,
		stagingBuffer.buffer, //stagging buffer
		localBuffer.buffer, //non stagging buffer
		1,
		&copyRegion);

	VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	VK_CHECK_RESULT(vkQueueSubmit(copyQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(copyQueue));
	vkFreeCommandBuffers(pRendInit.m_SwapChain.device, pRendInit.m_CmdPool, 1, &copyCmd);


	vkDestroyBuffer(pRendInit.m_SwapChain.device, stagingBuffer.buffer, nullptr);
	vkFreeMemory(pRendInit.m_SwapChain.device, stagingBuffer.memory, nullptr);


	if (enumDrawDescriptors == (drawVertFlags::Vertex | drawVertFlags::Normal | drawVertFlags::Uv))
	{
		BindVertexNormalUvDescriptor(localBuffer);
	}
	else if (enumDrawDescriptors == (drawVertFlags::Vertex | drawVertFlags::Normal | drawVertFlags::Uv | drawVertFlags::Tangent | drawVertFlags::Binormal))
	{
		BindVertexNormalUvTangentBinormalDescriptor(localBuffer);
	}
	return VK_SUCCESS;
}

VkResult VkBufferObject::SubmitCommandBuffer(const VkQueue& copyQueue, const VkCommandBuffer& copyCmd, const VulkanRendererInitializer& pRendInit)
{
	VK_CHECK_RESULT(vkEndCommandBuffer(copyCmd));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &copyCmd;

	VK_CHECK_RESULT(vkQueueSubmit(copyQueue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(copyQueue));
	vkFreeCommandBuffers(pRendInit.m_SwapChain.device, pRendInit.m_CmdPool, 1, &copyCmd);

	return VK_SUCCESS;
}

void VkBufferObject::DeleteBufferMemory(VkDevice device, VkBufferObject_s& buffer, const VkAllocationCallbacks* pAllocator)
{
	if(buffer.buffer != VK_NULL_HANDLE)
	{
		vkDestroyBuffer(device, buffer.buffer, nullptr);
		vkFreeMemory(device, buffer.memory, nullptr);
	}
}


void VkBufferObject::BindVertexDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawVert);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(1);
	// Location 0 : Position
	localBuffer.attributeDescriptions[0].binding = 0;
	localBuffer.attributeDescriptions[0].location = 0;
	localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[0].offset = offsetof(drawVert, vertex);


	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}


void VkBufferObject::BindNormalDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawVert);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(3);

	// Location 0 : Normal
	localBuffer.attributeDescriptions[1].binding = 0;
	localBuffer.attributeDescriptions[1].location = 0;
	localBuffer.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[1].offset = offsetof(drawVert, normal);


	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}


void VkBufferObject::BindUvDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawVert);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(1);

	// Location 0 : Uv
	localBuffer.attributeDescriptions[0].binding = 0;
	localBuffer.attributeDescriptions[0].location = 0;
	localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
	localBuffer.attributeDescriptions[0].offset = offsetof(drawVert, tex);

	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}


void VkBufferObject::BindVertexUvDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawFont);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(2);
	// Location 0 : Position
	localBuffer.attributeDescriptions[0].binding = 0;
	localBuffer.attributeDescriptions[0].location = 0;
	localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[0].offset = offsetof(drawFont, vertex);


	// Location 1 : Uv
	localBuffer.attributeDescriptions[1].binding = 0;
	localBuffer.attributeDescriptions[1].location = 1;
	localBuffer.attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
	localBuffer.attributeDescriptions[1].offset = offsetof(drawFont, tex);

	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}


void VkBufferObject::BindVertexNormalDescriptor(VkBufferObject_s& localBuffer)
{

}

void VkBufferObject::BindVertexNormalUvDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawVert);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(3);
	// Location 0 : Position
	localBuffer.attributeDescriptions[0].binding = 0;
	localBuffer.attributeDescriptions[0].location = 0;
	localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[0].offset = offsetof(drawVert, vertex);

	// Location 1 : Normal
	localBuffer.attributeDescriptions[1].binding = 0;
	localBuffer.attributeDescriptions[1].location = 1;
	localBuffer.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[1].offset = offsetof(drawVert, normal);

	// Location 2 : Uv
	localBuffer.attributeDescriptions[2].binding = 0;
	localBuffer.attributeDescriptions[2].location = 2;
	localBuffer.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	localBuffer.attributeDescriptions[2].offset = offsetof(drawVert, tex);


	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}

void VkBufferObject::BindVertexNormalUvTangentBinormalDescriptor(VkBufferObject_s& localBuffer)
{
	// Binding description
	localBuffer.bindingDescriptions.resize(1);
	localBuffer.bindingDescriptions[0].binding = 0;
	localBuffer.bindingDescriptions[0].stride = sizeof(drawVert);
	localBuffer.bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	// Attribute descriptions
	// Describes memory layout and shader attribute locations
	localBuffer.attributeDescriptions.resize(5);
	// Location 0 : Position
	localBuffer.attributeDescriptions[0].binding = 0;
	localBuffer.attributeDescriptions[0].location = 0;
	localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[0].offset = offsetof(drawVert, vertex);

	// Location 1 : Normal
	localBuffer.attributeDescriptions[1].binding = 0;
	localBuffer.attributeDescriptions[1].location = 1;
	localBuffer.attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[1].offset = offsetof(drawVert, normal);

	// Location 2 : Uv
	localBuffer.attributeDescriptions[2].binding = 0;
	localBuffer.attributeDescriptions[2].location = 2;
	localBuffer.attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
	localBuffer.attributeDescriptions[2].offset = offsetof(drawVert, tex);

	// Location 3 : Tangent
	localBuffer.attributeDescriptions[3].binding = 0;
	localBuffer.attributeDescriptions[3].location = 3;
	localBuffer.attributeDescriptions[3].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[3].offset = offsetof(drawVert, tangent);


	// Location 4 : Binormal
	localBuffer.attributeDescriptions[4].binding = 0;
	localBuffer.attributeDescriptions[4].location = 4;
	localBuffer.attributeDescriptions[4].format = VK_FORMAT_R32G32B32_SFLOAT;
	localBuffer.attributeDescriptions[4].offset = offsetof(drawVert, bitangent);



	// Assign to vertex input state
	localBuffer.inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	localBuffer.inputState.pNext = NULL;
	localBuffer.inputState.flags = VK_FLAGS_NONE;
	localBuffer.inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(localBuffer.bindingDescriptions.size());
	localBuffer.inputState.pVertexBindingDescriptions = localBuffer.bindingDescriptions.data();
	localBuffer.inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(localBuffer.attributeDescriptions.size());
	localBuffer.inputState.pVertexAttributeDescriptions = localBuffer.attributeDescriptions.data();
}



void VkBufferObject::BindVertUvBoneWeightBoneId(VkBufferObject_s& localBuffer)
{

}




void VkBufferObject::FreeMeshBufferResources(VkDevice& device, VkBufferObject_s& meshBuffer)
{
	vkDestroyBuffer(device, meshBuffer.buffer, nullptr);
	vkFreeMemory(device, meshBuffer.memory, nullptr);
}