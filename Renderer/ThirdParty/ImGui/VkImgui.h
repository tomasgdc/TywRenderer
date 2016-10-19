#pragma once


/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once


//forward declared
class GlyphData;
class VertexBuffer;
namespace VkTools
{
	struct VulkanTexutre;
	class  VulkanTextureLoader;
}

class TYWRENDERER_API VkImgui
{
public:
	VkImgui(VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkQueue queue,
		std::vector<VkFramebuffer> &framebuffers,
		VkFormat colorformat,
		VkFormat depthformat,
		uint32_t *framebufferwidth,
		uint32_t *framebufferheight);

	~VkImgui();


	void		 InitImgui(VulkanRendererInitializer *m_pWRenderer, bool install_callbacks);
	 void        NewFrame(double current_time);
	 void        BuildCommandBuffer(VkCommandBuffer command_buffer);
	 void		 BuildImgui();


	// Use if you want to reset your rendering device without losing ImGui state.
	 void        InvalidateFontUploadObjects();
	 void        InvalidateDeviceObjects();
	 bool        CreateFontTexture(VkCommandBuffer command_buffer);
	 bool        CreateDeviceObjects();

public:
	VkDevice					device;
	VkPipelineLayout			pipelineLayout;
	VkDescriptorSetLayout		descriptorSetLayout;
	VkDescriptorPool			descriptorPool;
	VkRenderPass				renderPass;
	VkPipelineCache				pipelineCache;
	VkPipeline					pipeline;
	VkCommandPool				commandPool;
	VkPhysicalDeviceMemoryProperties deviceMemoryProperties;
	VkQueue						queue;
	VkPhysicalDevice			physicalDevice;

	VkFormat					colorFormat;
	VkFormat					depthFormat;

	uint32_t *					frameBufferWidth;
	uint32_t *					frameBufferHeight;


	std::vector<VkShaderModule>   m_ShaderModules;
	std::vector<VkCommandBuffer>  cmdBuffers;
	std::vector<VkFramebuffer*>   frameBuffers;



	//Vertex
	VkBufferObject_s			m_BufferData;
	drawFont*				    pData;
	drawFont*					pDataLocal; //during text updates
	uint32_t					numLetters;


	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		float lodBias = 0.0f;
	}m_uboVS;


	struct
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	}  uniformDataVS;
public:
	GlyphData* data;
	std::unordered_map<char, VkTools::VulkanTexture*> glyphs; //Create hash map
	std::unordered_map<char, VkDescriptorSet> descriptors; //Create hash map
	std::string text;
};