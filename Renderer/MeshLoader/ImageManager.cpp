//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#include <RendererPch\stdafx.h>


//Renderer Includes
#include "Vulkan\VulkanTextureLoader.h"
#include "ImageManager.h"


ImageManager* globalImage(nullptr);

//TODO: Change
//Path to textures
std::string path = "../../../Assets/Textures/";


ImageManager::ImageManager(VkPhysicalDevice physicalDevice, VkDevice device, VkQueue queue, VkCommandPool cmdPool)
{
	m_VkTextureLoader = TYW_NEW VkTools::VulkanTextureLoader(physicalDevice, device, queue, cmdPool);
}


ImageManager::~ImageManager()
{
	delete m_VkTextureLoader;
	m_VkTextureLoader = nullptr;
}


VkTools::VulkanTexture* ImageManager::GetImage(const std::string& name, VkFormat format)
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
		//
		SAFE_DELETE(m_it->second);
	}
}