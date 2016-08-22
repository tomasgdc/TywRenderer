//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#pragma once
#include <External\vulkan\vulkan.h>




class TYWRENDERER_API VkTexture2D
{
public:
	VkTexture2D(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool);
	~VkTexture2D();

	void GenerateTexture(void * data, VkFormat format, uint32_t size,uint32_t width, uint32_t height, uint32_t miplevels, bool bForceLinear, VkImageUsageFlags imageUsageFlags);

private:
	VkSampler sampler;
	VkImage image;
	VkImageLayout imageLayout;
	VkDeviceMemory deviceMemory;
	VkImageView view;
	uint32_t width, height;
	uint32_t mipLevels;
	uint32_t layerCount;
	VkDescriptorImageInfo descriptor;

private:
	VkPhysicalDevice physicalDevice;
	VkDevice device;
	VkQueue queue;
	VkCommandBuffer cmdBuffer;
	VkCommandPool cmdPool;
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
};
