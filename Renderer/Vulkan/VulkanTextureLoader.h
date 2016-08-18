#pragma once
/*
* Texture loader for Vulkan
*
* Copyright (C) 2016 by Sascha Willems - www.saschawillems.de
*
* This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*
*	Modified by Tomas Mikalausklas
*/

#pragma once
#include <External\vulkan\vulkan.h>

namespace VkTools
{

	struct TYWRENDERER_API VulkanTexture
	{
		VkSampler sampler;
		VkImage image;
		VkImageLayout imageLayout;
		VkDeviceMemory deviceMemory;
		VkImageView view;
		uint32_t width, height;
		uint32_t mipLevels;
		uint32_t layerCount;
		VkDescriptorImageInfo descriptor;
	};

	class TYWRENDERER_API VulkanTextureLoader
	{
	private:
		VkPhysicalDevice physicalDevice;
		VkDevice device;
		VkQueue queue;
		VkCommandBuffer cmdBuffer;
		VkCommandPool cmdPool;
		VkPhysicalDeviceMemoryProperties deviceMemoryProperties;

		// Get appropriate memory type index for a memory allocation
		uint32_t getMemoryType(uint32_t typeBits, VkFlags properties);
	public:
#if defined(__ANDROID__)
		AAssetManager* assetManager = nullptr;
#endif
		// Load a 2D texture
		bool LoadTexture(const std::string& filename, VkFormat format, VulkanTexture *texture);

		// Load a 2D texture
		bool LoadTexture(const std::string& filename, VkFormat format, VulkanTexture *texture, bool forceLinear);

		// Load a 2D texture
		bool LoadTexture(const std::string& filename, VkFormat format, VulkanTexture *texture, bool forceLinear, VkImageUsageFlags imageUsageFlags);

		// Clean up vulkan resources used by a texture object
		void DestroyTexture(VulkanTexture& texture);

		VulkanTextureLoader(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool);

		~VulkanTextureLoader();

		// Load a cubemap texture (single file)
		bool LoadCubemap(std::string filename, VkFormat format, VulkanTexture *texture);

		// Load an array texture (single file)
		bool LoadTextureArray(std::string filename, VkFormat format, VulkanTexture *texture);
	};
}
