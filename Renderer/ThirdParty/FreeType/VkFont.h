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

class TYWRENDERER_API VkFont
{
	public:
		VkFont(VkPhysicalDevice physicalDevice,
			VkDevice device,
			VkQueue queue,
			std::vector<VkFramebuffer> &framebuffers,
			VkFormat colorformat,
			VkFormat depthformat,
			uint32_t *framebufferwidth,
			uint32_t *framebufferheight);

		//Loads freetype and creates font
		void CreateFontVk(const std::string& strTypeface, int point_size, int dpi);

		
		//Initializes chars and loads textures
		void InitializeChars(char* source, VkTools::VulkanTextureLoader& pTextureLoader);


		void Resize(uint32_t windowWidth, uint32_t windowHeight);
		void BeginTextUpdate();
		void EndTextUpdate();
		void SubmitToQueue(VkQueue queue, uint32_t bufferindex);

		void PrepareResources(uint32_t width, uint32_t height);
		void PrepareRenderPass();
		void PreparePipeline();
		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();

		void UpdateUniformBuffers(uint32_t windowWidth, uint32_t windowHeight, float zoom);
		void PrepareUniformBuffers();

		void BuildCommandBuffers();
		void UpdateCommandBuffers();

		//Add Text
		void AddText(float x, float y, float ws, float hs, const std::string& text);


		bool Release();
		~VkFont();
private:
	VkPipelineShaderStageCreateInfo VkFont::LoadShader(const std::string& fileName, VkShaderStageFlagBits stage);

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
		glm::vec4 viewPos;
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