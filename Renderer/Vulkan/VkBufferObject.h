/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include <External\vulkan\vulkan.h>

//forward declaration
class VulkanSwapChain;
class VulkanRendererInitializer;
enum drawVertFlags;

namespace VkTools
{
	struct UniformData;
};

struct  VkBufferObject_s
{
	VkBufferObject_s(): buffer(0), memory(0){}
	VkBuffer& operator ()() { return buffer; }

	VkPipelineVertexInputStateCreateInfo inputState;
	std::vector<VkVertexInputBindingDescription> bindingDescriptions;
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

	VkBuffer buffer;
	VkDeviceMemory memory;
};

/*
	Helper class creating Buffer Objects
*/
namespace VkBufferObject
{
	 VkResult CreateBuffer(const VulkanSwapChain& pSwapChain, VkPhysicalDeviceMemoryProperties& memoryProperties, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkBufferObject_s& bufferObject, void *data = nullptr);
	 VkResult CreateBuffer(const VulkanSwapChain& pSwapChain, VkPhysicalDeviceMemoryProperties& memoryProperties, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkTools::UniformData& uniformData, void *data = nullptr);

	 VkResult SubmitBufferObjects(const VkCommandBuffer& copyCmd, const VkQueue& copyQueue, const VulkanRendererInitializer& pRendInit, VkDeviceSize size, VkBufferObject_s& stagingBuffer, VkBufferObject_s& localBuffer, drawVertFlags enumDrawDescriptors);

	//You can use this function if command buffer was not submitted
	 VkResult SubmitCommandBuffer(const VkQueue& copyQueue,const VkCommandBuffer& copyCmd, const VulkanRendererInitializer& pRendInit);

	 void DeleteBufferMemory(VkDevice device, VkBufferObject_s& buffer, const VkAllocationCallbacks* pAllocator);

	//Vertex
	 void BindVertexDescriptor(VkBufferObject_s& localBuffer);

	//Normal
	 void BindNormalDescriptor(VkBufferObject_s& localBuffer);

	//Uv
	 void BindUvDescriptor(VkBufferObject_s& localBuffer);

	//Vertex, Uv
	 void BindVertexUvDescriptor(VkBufferObject_s& localBuffer);

	//Vertex, Normal
	 void BindVertexNormalDescriptor(VkBufferObject_s& localBuffer);

	//Vertex, Normal, Uv
	 void BindVertexNormalUvDescriptor(VkBufferObject_s& localBuffer);

	//Vertex, Norma, Uv, Tangent, Binormal
	 void BindVertexNormalUvTangentBinormalDescriptor(VkBufferObject_s& localBuffer);

	//Vertex, Uv, BoneWeight, BoneId
	 void BindVertUvBoneWeightBoneId(VkBufferObject_s& localBuffer);

	 void FreeMeshBufferResources(VkDevice& device, VkBufferObject_s& meshBuffer);
};