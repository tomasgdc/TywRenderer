/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Vulkan Includes
#include <External\vulkan\vulkan.h>

//Renderer Includes
#include "Vulkan\VulkanTextureLoader.h"
#include "Material.h"


/*
========================
Material
========================
*/
Material::Material():m_texture(nullptr) 
{

}

/*
========================
Material
========================
*/
Material::Material(std::string name, std::string path):m_texture(nullptr)
{

}

/*
========================
getWidth
========================
*/
unsigned long Material::getWidth() const {
	if (!m_texture)return 0;
	return m_texture->width;
}

/*
========================
getHeight
========================
*/
unsigned long Material::getHeight() const {
	if (!m_texture)return 0;
	return m_texture->height;
}

/*
========================
setLighting

maybe not the best name.....
kinda confusing
========================
*/
void Material::setLighting(GLXMATERIAL* material) {
	memcpy(&m_lighting, material, sizeof(material));
}

/*
========================
setTexture
========================
*/
void Material::setTexture(VkTools::VulkanTexture* texture, bool copy) {
	if (texture == nullptr) { return; }

	if (copy) 
	{
		memcpy(m_texture, texture, sizeof(texture));
	}
	else 
	{
		m_texture = texture;
	}
}

/*
========================
Clear
========================
*/
void Material::Clear(VkDevice device) 
{
	if (!m_texture)return;

	//Delete texture data from gpu
	vkDestroyImageView(device, m_texture->view, nullptr);
	vkDestroyImage(device, m_texture->image, nullptr);
	vkFreeMemory(device, m_texture->deviceMemory, nullptr);
	vkDestroySampler(device, m_texture->sampler, nullptr);

	//Delete 
	SAFE_DELETE(m_texture);
}

/*
========================
~Material
========================
*/
Material::~Material() 
{

}