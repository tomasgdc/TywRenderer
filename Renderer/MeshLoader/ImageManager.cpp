/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "Vulkan\VulkanTextureLoader.h"
#include "ImageManager.h"


ImageManager* globalImage(nullptr);


ImageManager::ImageManager(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool)
{
	m_VkTextureLoader = TYW_NEW VkTools::VulkanTextureLoader(physicalDevice, device, queue, cmdPool);
}


ImageManager::~ImageManager()
{
	//If there are images that were not deleted
	if (m_images.size() > 0)
	{
		PurgeAllImages();
	}

	delete m_VkTextureLoader;
	m_VkTextureLoader = nullptr;
}


VkTools::VulkanTexture* ImageManager::GetImage(const std::string& name, const std::string& path, VkFormat format)
{
	//find image by name
	m_it = m_images.find(name);

	//if not found, create new texture
	if (m_it == m_images.end()) 
	{
		VkTools::VulkanTexture* pTexture = TYW_NEW VkTools::VulkanTexture;
		
		//check if texture was loaded
		if (!m_VkTextureLoader->LoadTexture((path + name), format, pTexture))
		{
			printf("ERROR: Could not find texture %s\n", name.c_str());
			SAFE_DELETE(pTexture);
			return nullptr;
		}
		else 
		{
			InsertImage(pTexture, name);
			return pTexture;
		}
	}
	else 
	{
		printf("Material %s is already in the system \n", name.c_str());
		return m_it->second;
	}
}


void ImageManager::InsertImage(VkTools::VulkanTexture* image, const std::string& fileName) {
	if (image == nullptr) { return; }
	m_images.insert(map::value_type(fileName, image));
}

void ImageManager::PurgeAllImages() 
{
	for (m_it = m_images.begin(); m_it != m_images.end(); ++m_it) 
	{
		VkTools::VulkanTexture* image = m_it->second;
		if (image != nullptr)
		{
			m_VkTextureLoader->DestroyTexture(image);
			SAFE_DELETE(image);
		}
	}
	m_images.clear();
}