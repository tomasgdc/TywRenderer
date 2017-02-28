/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#include <External\vulkan\vulkan.h>

//forward declared
namespace VkTools
{
	class VulkanTexture;
	class VulkanTextureLoader;
}
typedef enum VkFormat;


class  ImageManager 
{
public:
	ImageManager(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPoo);
	~ImageManager();

	//looks for loaded image
	VkTools::VulkanTexture*					GetImage(const std::string& name, const std::string& path, VkFormat format);

	//Purges all images
	void									PurgeAllImages();
private:
	//Insert image into map
	void									InsertImage(VkTools::VulkanTexture* image, const std::string& fileName);

	typedef std::unordered_map<std::string, VkTools::VulkanTexture*>		map;
	map															m_images;
	map::iterator												m_it;


	VkTools::VulkanTextureLoader*								m_VkTextureLoader;
};

extern  ImageManager* globalImage;
