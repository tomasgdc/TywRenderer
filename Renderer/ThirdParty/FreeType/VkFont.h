//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
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
			//VkFormat colorformat,
			//VkFormat depthformat,
			uint32_t *framebufferwidth,
			uint32_t *framebufferheight);

		//Loads freetype and creates font
		void CreateFontVk(const std::string& strTypeface, int point_size, int dpi);

		
		//Initializes chars and loads textures
		void InitializeChars(char* source, VkTools::VulkanTextureLoader& pTextureLoader);



		void BeginTextUpdate();
		void EndTextUpdate();
		void SubmitToQueue(VkQueue queue, uint32_t bufferindex, VkSubmitInfo submitInfo);

		void PrepareResources();
		void PrepareRenderPass();
		void PreparePipeline();
		void SetupDescriptorSetLayout();
		void SetupDescriptorPool();
		void BuildCommandBuffers();
		void UpdateCommandBuffers();

		//Add Text
		void AddText(float x, float y, float ws, float hs, const char* text);


		bool Release();
		~VkFont();
private:
	VkPipelineShaderStageCreateInfo VkFont::LoadShader(const std::string& fileName, VkShaderStageFlagBits stage);

private:
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
private:
	GlyphData* data;
	std::unordered_map<char, VkTools::VulkanTexture*> glyphs; //Create hash map
	std::unordered_map<char, VkDescriptorSet> descriptors; //Create hash map
	std::string text;
};