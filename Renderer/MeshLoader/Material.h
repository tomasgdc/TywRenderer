/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once

//forward declared
namespace VkTools
{
	class VulkanTexture;
}

typedef struct  _GLXMATERIAL
{
	glm::vec4 color;
	glm::vec4 diffuse;
	glm::vec4 ambient;
	glm::vec4 specular;
	glm::vec4 emmisive;
	float power;
}GLXMATERIAL;


class  Material
{
public:
	Material();
	Material(std::string name, std::string path = "./");
	~Material();

				//Deletes texture2D class data
	void		Clear(VkDevice device);

				//sets texture data via memcpy
	void		setLighting(GLXMATERIAL* material);

				//sets texture data via memcpy if copy is true
				//if copy false, then it just copy the location of the data
	void		setTexture(VkTools::VulkanTexture* texture, bool copy);

	void		setTextureName(const std::string& strName) { m_strTextureName = strName; }

	VkTools::VulkanTexture*    getTexture() const { return m_texture; }
	GLXMATERIAL*  getLighting()  { return static_cast<GLXMATERIAL*> (&m_lighting); }

				  //returns texture width
	unsigned long getWidth() const;

				  //returns texutre height
	unsigned long getHeight() const;


	void SetAlpha(const float alpha) { m_lighting.diffuse.w = alpha; }
	bool HasAlpha() const { return GetAlpha() != 1.0f; }
	float GetAlpha() const { return m_lighting.diffuse.w; }

private:
	std::string					m_strTextureName;
	VkTools::VulkanTexture*		m_texture;
	GLXMATERIAL					m_lighting;
};
