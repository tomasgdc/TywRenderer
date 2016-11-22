/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <RendererPch\stdafx.h>
#include <iomanip>
#include <random> //std::uniform_real_distribution

//Triangle Includes
#include "Main.h"
#include <conio.h>

//Renderer Includes
#include <Renderer\VKRenderer.h>
#include <Renderer\Vulkan\VulkanTools.h>
#include <Renderer\Vulkan\VulkanTextureLoader.h>

//Model Loader Includes
#include <Renderer\MeshLoader\Model_local.h>
#include <Renderer\MeshLoader\ImageManager.h>
#include <Renderer\MeshLoader\Material.h>

//Vulkan Includes
#include <Renderer\Vulkan\VkBufferObject.h>
#include <Renderer\Vulkan\VulkanSwapChain.h>


//Font Rendering
#include <Renderer\ThirdParty\FreeType\VkFont.h>


//math
#include <External\glm\glm\gtc\matrix_inverse.hpp>

//Camera
#include <Renderer\Camera.h>

#include <Renderer\ThirdParty\ImGui\imgui.h>



//Global variables
//====================================================================================
#if defined(_WIN32)
#define KEY_ESCAPE VK_ESCAPE 
#define KEY_F1 VK_F1
#define KEY_F2 VK_F2
#define KEY_W 0x57
#define KEY_A 0x41
#define KEY_S 0x53
#define KEY_D 0x44
#define KEY_P 0x50
#define KEY_SPACE 0x20
#define KEY_KPADD 0x6B
#define KEY_KPSUB 0x6D
#define KEY_B 0x42
#define KEY_F 0x46
#define KEY_L 0x4C
#define KEY_N 0x4E
#define KEY_O 0x4F
#define KEY_T 0x54
#elif defined(__ANDROID__)
// Dummy key codes 
#define KEY_ESCAPE 0x0
#define KEY_F1 0x1
#define KEY_F2 0x2
#define KEY_W 0x3
#define KEY_A 0x4
#define KEY_S 0x5
#define KEY_D 0x6
#define KEY_P 0x7
#define KEY_SPACE 0x8
#define KEY_KPADD 0x9
#define KEY_KPSUB 0xA
#define KEY_B 0xB
#define KEY_F 0xC
#define KEY_L 0xD
#define KEY_N 0xE
#define KEY_O 0xF
#define KEY_T 0x10
#elif defined(__linux__)
#define KEY_ESCAPE 0x9
#define KEY_F1 0x43
#define KEY_F2 0x44
#define KEY_W 0x19
#define KEY_A 0x26
#define KEY_S 0x27
#define KEY_D 0x28
#define KEY_P 0x21
#define KEY_SPACE 0x41
#define KEY_KPADD 0x56
#define KEY_KPSUB 0x52
#define KEY_B 0x38
#define KEY_F 0x29
#define KEY_L 0x2E
#define KEY_N 0x39
#define KEY_O 0x20
#define KEY_T 0x1C
#endif

// todo: Android gamepad keycodes outside of define for now
#define GAMEPAD_BUTTON_A 0x1000
#define GAMEPAD_BUTTON_B 0x1001
#define GAMEPAD_BUTTON_X 0x1002
#define GAMEPAD_BUTTON_Y 0x1003
#define GAMEPAD_BUTTON_L1 0x1004
#define GAMEPAD_BUTTON_R1 0x1005
#define GAMEPAD_BUTTON_START 0x1006


uint32_t	g_iDesktopWidth = 0;
uint32_t	g_iDesktopHeight = 0;
bool		g_bPrepared = false;

glm::vec3	g_Rotation = glm::vec3();
glm::vec2	g_MousePos;


// Use to adjust mouse rotation speed
float		g_RotationSpeed = 1.0f;
// Use to adjust mouse zoom speed
float		g_ZoomSpeed = 1.0f;
float       g_zoom = 1.0f;

VkClearColorValue g_DefaultClearColor = { { 0.5f, 0.5f, 0.5f, 1.0f } };


#define VERTEX_BUFFER_BIND_ID 0
// Set to "true" to enable Vulkan's validation layers
// See vulkandebug.cpp for details
#define ENABLE_VALIDATION false
// Set to "true" to use staging buffers for uploading
// vertex and index data to device local memory
// See "prepareVertices" for details on what's staging
// and on why to use it
#define USE_STAGING true

#define GAMEPAD_BUTTON_A 0x1000
#define GAMEPAD_BUTTON_B 0x1001
#define GAMEPAD_BUTTON_X 0x1002
#define GAMEPAD_BUTTON_Y 0x1003
#define GAMEPAD_BUTTON_L1 0x1004
#define GAMEPAD_BUTTON_R1 0x1005
#define GAMEPAD_BUTTON_START 0x1006

// Framebuffer properties
#if defined(__ANDROID__)
#define GBUFF_DIM 1024
#else
#define GBUFF_DIM 2048
#endif
#define GBUFF_FILTER VK_FILTER_LINEAR



//====================================================================================


//functions
//====================================================================================

bool GenerateEvents(MSG& msg);
void WIN_Sizing(WORD side, RECT *rect);
LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//====================================================================================



class Renderer final : public VKRenderer
{
public:
	Camera  m_Camera;
	bool	m_bViewUpdated;
private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;

	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
		// Text overlay submission and execution
		VkSemaphore textOverlayComplete;
		//SSAO passs semaphore
		VkSemaphore ssaoSemaphore;
	} Semaphores;


	float zNear = 1.0f;
	float zFar = 128.0f;

	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 normal;
		glm::vec4 viewPos;
		glm::vec4 instancePos[3];
	}m_uboVS;

	uint32_t numVerts = 0;
	uint32_t numUvs = 0;
	uint32_t numNormals = 0;

	std::vector<VkBufferObject_s>	listLocalBuffers;
	std::vector<VkDescriptorSet>	listDescriptros;
	std::vector<uint32_t>			meshSize;




	// Framebuffer for offscreen rendering
	struct FrameBufferAttachment {
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
		VkFormat format;
	};
	struct FrameBuffer {
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment position;
		FrameBufferAttachment nm; //normal and diffuse
		FrameBufferAttachment ssao;
		FrameBufferAttachment depth;
		VkRenderPass renderPass;
	} offScreenFrameBuf;


	// One sampler for the frame buffer color attachments
	VkSampler colorSampler;

	struct
	{
		VkTools::UniformData mesh;
		VkTools::UniformData quad;
		VkTools::UniformData fsLights;
		VkTools::UniformData vsFullScreen;
		VkTools::UniformData ssao;
	}  uniformData;

	struct Light
	{
		glm::vec4 position;
		glm::vec3 color;
		float radius;
	};

	struct
	{
		Light lights[6];
		glm::vec4 viewPos;
	} uboFragmentLights;

	struct
	{
		std::array<glm::vec3, 64> ssaoKernel;
		glm::mat4 projection;
	}uboSSAO;


	struct
	{
		glm::mat4 projection;
		glm::mat4 model;
	} uboFullScreen;

	VkPipeline			frameBufferPipeline;
	VkCommandBuffer		GBufferScreenCmdBuffer = VK_NULL_HANDLE;
	VkSemaphore			GBufferSemaphore = VK_NULL_HANDLE;
	VkPipelineLayout	frameBufferPipelineLayout;
	VkDescriptorSet		frameBufferDescriptorSet;

	struct mesh
	{
		VkBufferObject_s				vertex;
		VkBufferObject_s				index;
		uint32_t						numIndexes;
	}quadMesh;

	//plane and quad
	VkPipeline quadPipeline;
	VkPipelineLayout quadPipelineLayout;
	VkDescriptorSet  quadDescriptorSet;
	VkDescriptorSet  defferedModelDescriptorSet;

	//SSAO
	VkPipeline ssaoPipeline;
	VkPipelineLayout ssaoPipelineLayout;
	VkDescriptorSetLayout ssaoDescriptorSetLayout;
	VkDescriptorSet  ssaoDescriptorSet;
	VkCommandBuffer	 ssaoCmdBuffer = VK_NULL_HANDLE;
	VkTools::VulkanTexture m_NoiseGeneratedTexture;

	struct {
		glm::mat4 mvp;
	} quadUniformData;

	RenderModelStatic staticModel;
	VkCommandBuffer  cmdGui;
public:
	Renderer();
	~Renderer();

	void BuildCommandBuffers() override;
	void UpdateUniformBuffers() override;
	void PrepareUniformBuffers() override;
	void PrepareVertices(bool useStagingBuffers) override;
	void VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling, bool bUseCubeMap = false) override;
	void PreparePipeline() override;
	void SetupDescriptorSet() override;
	void SetupDescriptorSetLayout() override;
	void SetupDescriptorPool() override;
	void LoadAssets() override;
	void PrepareSemaphore() override;
	void StartFrame() override;
	void LoadGUI() override;

	void GenerateTexture(std::vector<glm::vec3> &ImageData, uint32_t imageX, uint32_t imageY, uint32_t mipMapLevel, VkFormat format, VkTools::VulkanTexture *texture, VkImageUsageFlags imageUsageFlags); //Add to TywRenderer DLL
	void InitializeSSAOData();
	void ChangeLodBias(float delta);
	void BeginTextUpdate();
	void CreateFrameBuffer();
	void CreateAttachement(VkFormat format, VkImageUsageFlagBits usage, FrameBufferAttachment *attachment, VkCommandBuffer layoutCmd); //Add to TywRendere DLL

	void PrepareFramebufferCommands();
	void PrepareSSAOCommands();
	void PrepareMainRendererCommands();

	void GenerateQuad();
	void UpdateQuadUniformData(const glm::vec3& pos = glm::vec3(1.0, 1.0, 0.0));
	void UpdateUniformBuffersLights();
	void ImguiRender();
};

Renderer::Renderer() :m_bViewUpdated(false)
{
	m_Camera.type = Camera::CameraType::firstperson;
	m_Camera.movementSpeed = 5.0f;
#ifndef __ANDROID__
	m_Camera.rotationSpeed = 0.25f;
#endif
	m_Camera.position = { 0.47f, -3.8f, -3.34f };
	m_Camera.setRotation(glm::vec3(-164.0f, -164.0f, 0.0f));
	m_Camera.setPerspective(60.0f, (float)1280 / (float)720, 0.1f, 256.0f);
}

Renderer::~Renderer()
{
	//Destroy noise generated texture
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, m_NoiseGeneratedTexture.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, m_NoiseGeneratedTexture.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, m_NoiseGeneratedTexture.deviceMemory, nullptr);
	vkDestroySampler(m_pWRenderer->m_SwapChain.device, m_NoiseGeneratedTexture.sampler, nullptr);


	vkDestroySampler(m_pWRenderer->m_SwapChain.device, colorSampler, nullptr);

	//Destroy model data
	staticModel.Clear(m_pWRenderer->m_SwapChain.device);


	//Delete memory
	for (int i = 0; i < listLocalBuffers.size(); i++)
	{
		VkBufferObject::DeleteBufferMemory(m_pWRenderer->m_SwapChain.device, listLocalBuffers[i], nullptr);
	}


	//Destroy Shader Module
	for (int i = 0; i < m_ShaderModules.size(); i++)
	{
		vkDestroyShaderModule(m_pWRenderer->m_SwapChain.device, m_ShaderModules[i], nullptr);
	}

	//Uniform Data
	vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, uniformData.mesh.buffer, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, uniformData.mesh.memory, nullptr);
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.quad);
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.fsLights);
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.vsFullScreen);
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.ssao);

	//QUad
	VkBufferObject::DeleteBufferMemory(m_pWRenderer->m_SwapChain.device, quadMesh.vertex, nullptr);
	VkBufferObject::DeleteBufferMemory(m_pWRenderer->m_SwapChain.device, quadMesh.index, nullptr);

	//Position
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.position.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.position.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.position.mem, nullptr);

	//Normal and Diffuse
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.nm.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.nm.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.nm.mem, nullptr);


	//SSAO
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.ssao.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.ssao.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.ssao.mem, nullptr);

	//Depth
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.mem, nullptr);


	//Destroy framebuffer
	vkDestroyFramebuffer(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.frameBuffer, nullptr);


	//Destroy FrameBufferPipeline
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, frameBufferPipeline, nullptr);
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, quadPipeline, nullptr);
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, pipeline, nullptr);
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, ssaoPipeline, nullptr);

	//Destroy PipelineLayout
	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, frameBufferPipelineLayout, nullptr);
	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, pipelineLayout, nullptr);
	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, ssaoPipelineLayout, nullptr);

	//Destroy layout
	vkDestroyDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, descriptorSetLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, ssaoDescriptorSetLayout, nullptr);

	//Destroy RenderPass
	vkDestroyRenderPass(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.renderPass, nullptr);

	//Destroy Command Buffer and Semaphore
	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &GBufferScreenCmdBuffer);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, GBufferSemaphore, nullptr);

	//ssao
	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &ssaoCmdBuffer);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.ssaoSemaphore, nullptr);

	//Release semaphores
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.renderComplete, nullptr);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.textOverlayComplete, nullptr);
}

void Renderer::PrepareSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;

	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.presentComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.renderComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.textOverlayComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.ssaoSemaphore));
}


void Renderer::ImguiRender()
{
	ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
	ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
}

void Renderer::InitializeSSAOData()
{
	std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f); // random floats between 0.0 - 1.0
	std::default_random_engine generator;
	for (uint32_t i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator) * 2.0f - 1.0f,
			randomFloats(generator)
		);
		sample = glm::normalize(sample);
		sample *= randomFloats(generator);
		float scale = static_cast<float>(i) / 64.0f;
		uboSSAO.ssaoKernel[i] = sample;
	}

	uboSSAO.projection = m_Camera.matrices.perspective;


	std::vector<glm::vec3> ssaoNoise;
	ssaoNoise.reserve(16);
	for (uint32_t i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0,
			randomFloats(generator) * 2.0 - 1.0,
			0.0f);
		ssaoNoise.push_back(noise);
	}

	//Generates texture. But cant see anything in debugger in it. Better generate on gpu... at the moment
	GenerateTexture(ssaoNoise, 4, 4, 1, VK_FORMAT_R16G16B16_SFLOAT, &m_NoiseGeneratedTexture, VK_IMAGE_USAGE_SAMPLED_BIT);
}

void Renderer::GenerateTexture(std::vector<glm::vec3> &tex2D, uint32_t imageX, uint32_t imageY, uint32_t mipMapLevel, VkFormat format, VkTools::VulkanTexture *texture, VkImageUsageFlags imageUsageFlags)
{
	texture->width = imageX;
	texture->height = imageY;
	texture->mipLevels = mipMapLevel;

	// Get device properites for the requested texture format
	VkFormatProperties formatProperties;
	vkGetPhysicalDeviceFormatProperties(m_pWRenderer->m_SwapChain.physicalDevice, format, &formatProperties);


	VkMemoryAllocateInfo memAllocInfo = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;


	// Use a separate command buffer for texture generation
	VkCommandBuffer cmdBuffer;
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		VkTools::Initializer::CommandBufferAllocateInfo(
			m_pWRenderer->m_CmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &cmdBuffer));
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));


	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

	VkBufferCreateInfo bufferCreateInfo = VkTools::Initializer::BufferCreateInfo();
	bufferCreateInfo.size = tex2D.size() * sizeof(glm::vec3);

	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferCreateInfo, nullptr, &stagingBuffer));

	// Get memory requirements for the staging buffer (alignment, memory type bits)
	vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, stagingBuffer, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);

	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAllocInfo, nullptr, &stagingMemory));
	VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, stagingBuffer, stagingMemory, 0));

	// Copy texture data into staging buffer
	uint8_t *data;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, stagingMemory, 0, memReqs.size, 0, (void **)&data));
	memcpy(data, tex2D.data(), tex2D.size() * sizeof(glm::vec3));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, stagingMemory);

	// Setup buffer copy regions for each mip level
	std::vector<VkBufferImageCopy> bufferCopyRegions;
	uint32_t offset = 0;

	for (uint32_t i = 0; i < texture->mipLevels; i++)
	{
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = i;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = static_cast<uint32_t>(imageX); //not working properly for mip level
		bufferCopyRegion.imageExtent.height = static_cast<uint32_t>(imageY); //not working properly for mip level

		bufferCopyRegion.imageExtent.depth = 1;
		bufferCopyRegion.bufferOffset = offset;

		bufferCopyRegions.push_back(bufferCopyRegion);
		offset += static_cast<uint32_t>(tex2D.size() * sizeof(glm::vec3)); //not working properly for mip level
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = VkTools::Initializer::ImageCreateInfo();
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = format;
	imageCreateInfo.mipLevels = texture->mipLevels;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = imageUsageFlags;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { texture->width, texture->height, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VK_CHECK_RESULT(vkCreateImage(m_pWRenderer->m_SwapChain.device, &imageCreateInfo, nullptr, &texture->image));

	vkGetImageMemoryRequirements(m_pWRenderer->m_SwapChain.device, texture->image, &memReqs);

	memAllocInfo.allocationSize = memReqs.size;

	memAllocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAllocInfo, nullptr, &texture->deviceMemory));
	VK_CHECK_RESULT(vkBindImageMemory(m_pWRenderer->m_SwapChain.device, texture->image, texture->deviceMemory, 0));

	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = texture->mipLevels;
	subresourceRange.layerCount = 1;

	// Image barrier for optimal image (target)
	// Optimal image will be used as destination for the copy
	VkTools::SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		subresourceRange);

	// Copy mip levels from staging buffer
	vkCmdCopyBufferToImage(
		cmdBuffer,
		stagingBuffer,
		texture->image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		static_cast<uint32_t>(bufferCopyRegions.size()),
		bufferCopyRegions.data()
	);

	// Change texture image layout to shader read after all mip levels have been copied
	texture->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	VkTools::SetImageLayout(
		cmdBuffer,
		texture->image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		texture->imageLayout,
		subresourceRange);

	// Submit command buffer containing copy and image layout commands
	VK_CHECK_RESULT(vkEndCommandBuffer(cmdBuffer));

	// Create a fence to make sure that the copies have finished before continuing
	VkFence copyFence;
	VkFenceCreateInfo fenceCreateInfo = VkTools::Initializer::FenceCreateInfo(VK_FLAGS_NONE);
	VK_CHECK_RESULT(vkCreateFence(m_pWRenderer->m_SwapChain.device, &fenceCreateInfo, nullptr, &copyFence));

	VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmdBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, copyFence));
	VK_CHECK_RESULT(vkWaitForFences(m_pWRenderer->m_SwapChain.device, 1, &copyFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

	vkDestroyFence(m_pWRenderer->m_SwapChain.device, copyFence, nullptr);

	// Clean up staging resources
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, stagingMemory, nullptr);
	vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, stagingBuffer, nullptr);


	// Create sampler
	VkSamplerCreateInfo sampler = {};
	sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	// Max level-of-detail should match mip level count
	sampler.maxLod = texture->mipLevels;
	// Enable anisotropic filtering
	sampler.maxAnisotropy = 8;
	sampler.anisotropyEnable = VK_TRUE;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(m_pWRenderer->m_SwapChain.device, &sampler, nullptr, &texture->sampler));

	// Create image view
	// Textures are not directly accessed by the shaders and
	// are abstracted by image views containing additional
	// information and sub resource ranges
	VkImageViewCreateInfo view = {};
	view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view.pNext = NULL;
	view.image = VK_NULL_HANDLE;
	view.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view.format = format;
	view.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// Linear tiling usually won't support mip maps
	// Only set mip map count if optimal tiling is used
	view.subresourceRange.levelCount = texture->mipLevels;
	view.image = texture->image;
	VK_CHECK_RESULT(vkCreateImageView(m_pWRenderer->m_SwapChain.device, &view, nullptr, &texture->view));

	// Fill descriptor image info that can be used for setting up descriptor sets
	texture->descriptor.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	texture->descriptor.imageView = texture->view;
	texture->descriptor.sampler = texture->sampler;
//	return true;
}


void Renderer::UpdateUniformBuffersLights()
{
	timer += timerSpeed * frameTimer;
	if (timer > 1.0)
	{
		timer -= 1.0f;
	}


	// White
	uboFragmentLights.lights[0].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[0].color = glm::vec3(1.5f);
	uboFragmentLights.lights[0].radius = 25.0f;
	// Red
	uboFragmentLights.lights[1].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[1].color = glm::vec3(1.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[1].radius = 25.0f;
	// Blue
	uboFragmentLights.lights[2].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[2].color = glm::vec3(0.0f, 0.0f, 2.5f);
	uboFragmentLights.lights[2].radius = 25.0f;
	// Yellow
	uboFragmentLights.lights[3].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[3].color = glm::vec3(1.0f, 1.0f, 0.0f);
	uboFragmentLights.lights[3].radius = 25.0f;
	// Green
	uboFragmentLights.lights[4].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[4].color = glm::vec3(0.0f, 1.0f, 0.2f);
	uboFragmentLights.lights[4].radius = 25.0f;
	// Yellow
	uboFragmentLights.lights[5].position = glm::vec4(0.0f, -4.0f, 0.0f, 0.0f);
	uboFragmentLights.lights[5].color = glm::vec3(1.0f, 0.7f, 0.3f);
	uboFragmentLights.lights[5].radius = 25.0f;

	//Update lights
	uboFragmentLights.lights[0].position.x = -4.0f + sin(glm::radians(-360.0f * timer)) * 5.0f;
	uboFragmentLights.lights[0].position.z = 0.0f - cos(glm::radians(-360.0f * timer)) * 5.0f;

	uboFragmentLights.lights[1].position.x = -4.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 5.0f;
	uboFragmentLights.lights[1].position.z = 0.0f + cos(glm::radians(360.0f * timer) + 45.0f) * 5.0f;

	uboFragmentLights.lights[2].position.x = -4.0f + sin(glm::radians(-360.0f * timer + 45.0f)) * 5.0f;
	uboFragmentLights.lights[2].position.z = 0.0f + cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

	uboFragmentLights.lights[3].position.x = -4.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 5.0f;
	uboFragmentLights.lights[3].position.x = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

	uboFragmentLights.lights[4].position.x = -4.0f + sin(glm::radians(-360.0f * timer + 90.0f)) * 5.0f;
	uboFragmentLights.lights[4].position.z = 0.0f - cos(glm::radians(360.0f * timer + 45.0f)) * 5.0f;

	uboFragmentLights.lights[5].position.x = -4.0f + sin(glm::radians(-360.0f * timer + 135.0f)) * 5.0f;
	uboFragmentLights.lights[5].position.z = 0.0f - cos(glm::radians(-360.0f * timer - 45.0f)) * 5.0f;

	// Current view position
	uboFragmentLights.viewPos = glm::vec4(m_Camera.position, 0.0f) * glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f);

	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.fsLights.memory, 0, sizeof(uboFragmentLights), 0, (void **)&pData));
	memcpy(pData, &uboFragmentLights, sizeof(uboFragmentLights));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.fsLights.memory);
}

void Renderer::LoadGUI()
{
	//Load Imgui
	ImGui_ImplGlfwVulkan_Init(m_pWRenderer, &m_WindowWidth, &m_WindowHeight,
		GetAssetPath() + "Shaders/ImGui/text.vert.spv",
		GetAssetPath() + "Shaders/ImGui/text.frag.spv",
		false);

	//Create command buffer
	VkCommandBuffer cmd;
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		VkTools::Initializer::CommandBufferAllocateInfo(
			m_pWRenderer->m_CmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &cmd));
	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(cmd, &cmdBufInfo));

	ImGui_ImplGlfwVulkan_CreateFontsTexture(cmd);

	VK_CHECK_RESULT(vkEndCommandBuffer(cmd));
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &cmd;

	VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));
	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &cmd);
	ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
}

void Renderer::GenerateQuad()
{
#define NUM_QUADS 6
	std::vector<drawVert> vertData;
	vertData.reserve(NUM_QUADS * 4);

	float x = 0.0f;
	float y = 0.0f;

	// Last component of normal is used for debug display sampler index
	for (uint32_t i = 0; i < NUM_QUADS; i++)
	{
		vertData.push_back({ glm::vec3(x + 1.0f, y + 1.0f, 0.0f),	glm::vec3(0.0f, 0.0f, static_cast<float>(i)),	glm::vec2(1.0f,1.0f) });
		vertData.push_back({ glm::vec3(x,      y + 1.0f, 0.0f),     glm::vec3(0.0f, 0.0f, static_cast<float>(i)),	glm::vec2(0.0f,1.0f) });
		vertData.push_back({ glm::vec3(x, y, 0.0f),					glm::vec3(0.0f, 0.0f, static_cast<float>(i)),  glm::vec2(0.0f, 0.0f) });
		vertData.push_back({ glm::vec3(x + 1.0f, y, 0.0f),			glm::vec3(0.0f, 0.0f, static_cast<float>(i)),  glm::vec2(1.0f, 0.0f) });

		x += 1.0f;
		if (x > 1.0f)
		{
			x = 0.0f;
			y += 1.0f;
		}
	}

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0,1,2, 2,3,0 };
	for (uint32_t i = 0; i < NUM_QUADS; ++i)
	{
		uint32_t indices[6] = { 0,1,2, 2,3,0 };
		for (auto index : indices)
		{
			indexBuffer.push_back(i * 4 + index);
		}
	}
	quadMesh.numIndexes = static_cast<uint32_t>(indexBuffer.size());


	VkBufferObject_s staggingBufferVbo;
	VkBufferObject_s staggingIndexBufferVbo;

	//Create stagign buffer
	//Verts
	VkBufferObject::CreateBuffer(
		m_pWRenderer->m_SwapChain,
		m_pWRenderer->m_DeviceMemoryProperties,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		static_cast<uint32_t>(sizeof(drawVert) * vertData.size()),
		staggingBufferVbo,
		vertData.data());

	//Index
	VkBufferObject::CreateBuffer(
		m_pWRenderer->m_SwapChain,
		m_pWRenderer->m_DeviceMemoryProperties,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
		static_cast<uint32_t>(sizeof(uint32_t) * indexBuffer.size()),
		staggingIndexBufferVbo,
		indexBuffer.data());


	//Create Local Copy
	//Verts
	VkBufferObject::CreateBuffer(
		m_pWRenderer->m_SwapChain,
		m_pWRenderer->m_DeviceMemoryProperties,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		static_cast<uint32_t>(sizeof(drawVert) * vertData.size()),
		quadMesh.vertex);

	//Index
	VkBufferObject::CreateBuffer(
		m_pWRenderer->m_SwapChain,
		m_pWRenderer->m_DeviceMemoryProperties,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		static_cast<uint32_t>(sizeof(uint32_t) * indexBuffer.size()),
		quadMesh.index);


	//Create new command buffer
	VkCommandBuffer copyCmd = GetCommandBuffer(true);


	//Submit info to the queue
	VkBufferObject::SubmitBufferObjects(
		copyCmd,
		m_pWRenderer->m_Queue,
		*m_pWRenderer,
		static_cast<uint32_t>(sizeof(drawVert) * vertData.size()),
		staggingBufferVbo,
		quadMesh.vertex, (drawVertFlags::Vertex | drawVertFlags::Uv | drawVertFlags::Normal));


	copyCmd = GetCommandBuffer(true);
	VkBufferObject::SubmitBufferObjects(
		copyCmd,
		m_pWRenderer->m_Queue,
		*m_pWRenderer,
		static_cast<uint32_t>(sizeof(uint32_t) * indexBuffer.size()),
		staggingIndexBufferVbo,
		quadMesh.index, drawVertFlags::None);
}

void Renderer::UpdateQuadUniformData(const glm::vec3& pos)
{
	static glm::mat4 mvp = glm::ortho(0.0f, 5.0f, 0.0f, 5.0f, -1.0f, 1.0f)  * glm::mat4();
	quadUniformData.mvp = mvp;

	// Map uniform buffer and update it
	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.quad.memory, 0, sizeof(quadUniformData), 0, (void **)&pData));
	memcpy(pData, &quadUniformData, sizeof(quadUniformData));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.quad.memory);
}

void Renderer::PrepareSSAOCommands()
{
	if (ssaoCmdBuffer == VK_NULL_HANDLE)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			VkTools::Initializer::CommandBufferAllocateInfo(
				m_pWRenderer->m_CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &ssaoCmdBuffer));
	}

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	std::array<VkClearValue, 2> clearValues;
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = VkTools::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = offScreenFrameBuf.renderPass;
	renderPassBeginInfo.framebuffer = offScreenFrameBuf.frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = offScreenFrameBuf.width;
	renderPassBeginInfo.renderArea.extent.height = offScreenFrameBuf.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	VK_CHECK_RESULT(vkBeginCommandBuffer(ssaoCmdBuffer, &cmdBufInfo));


	VkViewport viewport = VkTools::Initializer::Viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
	vkCmdSetViewport(GBufferScreenCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = VkTools::Initializer::Rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
	vkCmdSetScissor(GBufferScreenCmdBuffer, 0, 1, &scissor);


	vkCmdBeginRenderPass(ssaoCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(ssaoCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipeline);
	vkCmdBindDescriptorSets(GBufferScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, ssaoPipelineLayout, 0, 1, &ssaoDescriptorSet, 0, NULL);

	vkCmdEndRenderPass(ssaoCmdBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(ssaoCmdBuffer));
}

void Renderer::PrepareMainRendererCommands()
{
	VkCommandBufferBeginInfo cmdBufInfo = {};
	cmdBufInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	cmdBufInfo.pNext = NULL;

	// Set clear values for all framebuffer attachments with loadOp set to clear
	// We use two attachments (color and depth) that are cleared at the 
	// start of the subpass and as such we need to set clear values for both
	VkClearValue clearValues[2];
	clearValues[0].color = g_DefaultClearColor;
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = {};
	renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBeginInfo.pNext = NULL;
	renderPassBeginInfo.renderPass = m_pWRenderer->m_RenderPass;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = g_iDesktopWidth;
	renderPassBeginInfo.renderArea.extent.height = g_iDesktopHeight;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	for (int32_t i = 0; i < m_pWRenderer->m_DrawCmdBuffers.size(); ++i)
	{

		// Set target frame buffer
		renderPassBeginInfo.framebuffer = m_pWRenderer->m_FrameBuffers[i];

		VK_CHECK_RESULT(vkBeginCommandBuffer(m_pWRenderer->m_DrawCmdBuffers[i], &cmdBufInfo));

		// Start the first sub pass specified in our default render pass setup by the base class
		// This will clear the color and depth attachment
		vkCmdBeginRenderPass(m_pWRenderer->m_DrawCmdBuffers[i], &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Update dynamic viewport state
		VkViewport viewport = {};
		viewport.height = (float)g_iDesktopHeight;
		viewport.width = (float)g_iDesktopWidth;
		viewport.minDepth = (float) 0.0f;
		viewport.maxDepth = (float) 1.0f;
		vkCmdSetViewport(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = g_iDesktopWidth;
		scissor.extent.height = g_iDesktopHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &scissor);
		VkDeviceSize offsets[1] = { 0 };

		// Final composition as full screen quad
		{
			vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &defferedModelDescriptorSet, 0, NULL);
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &quadMesh.vertex.buffer, offsets);
			vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], quadMesh.index.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], 6, 1, 0, 0, 1);
		}

		//quad debug
		{
			vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipeline);
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &quadDescriptorSet, 0, NULL);
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &quadMesh.vertex.buffer, offsets);
			vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], quadMesh.index.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], quadMesh.numIndexes, 1, 0, 0, 1);

			//			viewport.x = viewport.width * 0.5f;
			//			viewport.y = viewport.height * 0.5f;
			//			vkCmdSetViewport(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &viewport);
		}

		vkCmdEndRenderPass(m_pWRenderer->m_DrawCmdBuffers[i]);

		// Add a present memory barrier to the end of the command buffer
		// This will transform the frame buffer color attachment to a
		// new layout for presenting it to the windowing system integration 
		VkImageMemoryBarrier prePresentBarrier = {};
		prePresentBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		prePresentBarrier.pNext = NULL;
		prePresentBarrier.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		prePresentBarrier.dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
		prePresentBarrier.oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		prePresentBarrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		prePresentBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		prePresentBarrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
		prePresentBarrier.image = m_pWRenderer->m_SwapChain.buffers[i].image;

		VkImageMemoryBarrier *pMemoryBarrier = &prePresentBarrier;
		vkCmdPipelineBarrier(
			m_pWRenderer->m_DrawCmdBuffers[i],
			VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
			VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
			VK_FLAGS_NONE,
			0, nullptr,
			0, nullptr,
			1, &prePresentBarrier);

		VK_CHECK_RESULT(vkEndCommandBuffer(m_pWRenderer->m_DrawCmdBuffers[i]));

	}
}

void Renderer::PrepareFramebufferCommands()
{
	if (GBufferScreenCmdBuffer == VK_NULL_HANDLE)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			VkTools::Initializer::CommandBufferAllocateInfo(
				m_pWRenderer->m_CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &GBufferScreenCmdBuffer));
	}

	// Create a semaphore used to synchronize offscreen rendering and usage
	VkSemaphoreCreateInfo semaphoreCreateInfo = VkTools::Initializer::SemaphoreCreateInfo();
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &GBufferSemaphore));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	std::array<VkClearValue, 4> clearValues;
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[2].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[3].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = VkTools::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = offScreenFrameBuf.renderPass;
	renderPassBeginInfo.framebuffer = offScreenFrameBuf.frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = offScreenFrameBuf.width;
	renderPassBeginInfo.renderArea.extent.height = offScreenFrameBuf.height;
	renderPassBeginInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassBeginInfo.pClearValues = clearValues.data();

	VK_CHECK_RESULT(vkBeginCommandBuffer(GBufferScreenCmdBuffer, &cmdBufInfo));


	VkViewport viewport = VkTools::Initializer::Viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
	vkCmdSetViewport(GBufferScreenCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = VkTools::Initializer::Rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
	vkCmdSetScissor(GBufferScreenCmdBuffer, 0, 1, &scissor);


	vkCmdBeginRenderPass(GBufferScreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(GBufferScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, frameBufferPipeline);

	// Bind triangle vertex buffer (contains position and colors)
	VkDeviceSize offsets[1] = { 0 };
	for (int j = 0; j < listLocalBuffers.size(); j++)
	{
		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(GBufferScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, frameBufferPipelineLayout, 0, 1, &listDescriptros[j], 0, NULL);

		//Bind Buffer
		vkCmdBindVertexBuffers(GBufferScreenCmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &listLocalBuffers[j].buffer, offsets);

		//Draw
		//vkCmdDrawIndirect(GBufferScreenCmdBuffer, nullptr , 0, 3, 0);
		vkCmdDraw(GBufferScreenCmdBuffer, meshSize[j], 3, 0, 0);
	}
	vkCmdEndRenderPass(GBufferScreenCmdBuffer);
	VK_CHECK_RESULT(vkEndCommandBuffer(GBufferScreenCmdBuffer));
}



void Renderer::CreateAttachement(
	VkFormat format,
	VkImageUsageFlagBits usage,
	FrameBufferAttachment *attachment,
	VkCommandBuffer layoutCmd)
{
	VkImageAspectFlags aspectMask = 0;
	VkImageLayout imageLayout;
	attachment->format = format;

	if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	}
	if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
		imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	}


	assert(aspectMask > 0);

	VkImageCreateInfo image = VkTools::Initializer::ImageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = format;
	image.extent.width = offScreenFrameBuf.width;
	image.extent.height = offScreenFrameBuf.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	image.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkMemoryAllocateInfo memAlloc = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VK_CHECK_RESULT(vkCreateImage(m_pWRenderer->m_SwapChain.device, &image, nullptr, &attachment->image));
	vkGetImageMemoryRequirements(m_pWRenderer->m_SwapChain.device, attachment->image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);

	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &attachment->mem));
	VK_CHECK_RESULT(vkBindImageMemory(m_pWRenderer->m_SwapChain.device, attachment->image, attachment->mem, 0));

	VkImageViewCreateInfo imageView = VkTools::Initializer::ImageViewCreateInfo();
	imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageView.format = format;
	imageView.subresourceRange = {};
	imageView.subresourceRange.aspectMask = aspectMask;
	imageView.subresourceRange.baseMipLevel = 0;
	imageView.subresourceRange.levelCount = 1;
	imageView.subresourceRange.baseArrayLayer = 0;
	imageView.subresourceRange.layerCount = 1;
	imageView.image = attachment->image;
	VK_CHECK_RESULT(vkCreateImageView(m_pWRenderer->m_SwapChain.device, &imageView, nullptr, &attachment->view));
}


void Renderer::CreateFrameBuffer()
{

	//Create command buffer
	VkCommandBuffer layoutCmd;
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
		VkTools::Initializer::CommandBufferAllocateInfo(
			m_pWRenderer->m_CmdPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &layoutCmd));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	//Command buffer created and started
	VK_CHECK_RESULT(vkBeginCommandBuffer(layoutCmd, &cmdBufInfo));


	offScreenFrameBuf.width = GBUFF_DIM;
	offScreenFrameBuf.height = GBUFF_DIM;

	//Normal and diffuse
	CreateAttachement(VK_FORMAT_R32G32B32A32_UINT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.nm,
		layoutCmd);

	//Position
	CreateAttachement(VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.position,
		layoutCmd);

	//SSAO
	CreateAttachement(VK_FORMAT_R16G16B16A16_SFLOAT,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		&offScreenFrameBuf.ssao,
		layoutCmd);



	// Find a suitable depth format
	VkFormat attDepthFormat;
	VkBool32 validDepthFormat = VkTools::GetSupportedDepthFormat(m_pWRenderer->m_SwapChain.physicalDevice, &attDepthFormat);
	assert(validDepthFormat);

	//Depth
	CreateAttachement(attDepthFormat,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		&offScreenFrameBuf.depth,
		layoutCmd);

	//Free command buffer
	VK_CHECK_RESULT(vkEndCommandBuffer(layoutCmd));
	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &layoutCmd;

	VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));
	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &layoutCmd);

	// Set up separate renderpass with references
	// to the color and depth attachments

	std::array<VkAttachmentDescription, 4> attachmentDescs = {};

	// Init attachment properties
	for (uint32_t i = 0; i < attachmentDescs.size(); ++i)
	{
		attachmentDescs[i].samples = VK_SAMPLE_COUNT_1_BIT;
		attachmentDescs[i].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		attachmentDescs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachmentDescs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachmentDescs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		if (i == 3)
		{
			//depth buffer
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		}
		else
		{
			attachmentDescs[i].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachmentDescs[i].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		}
	}

	// Formats
	attachmentDescs[0].format = offScreenFrameBuf.position.format;
	attachmentDescs[1].format = offScreenFrameBuf.nm.format;
	attachmentDescs[2].format = offScreenFrameBuf.ssao.format;
	attachmentDescs[3].format = offScreenFrameBuf.depth.format;

	std::vector<VkAttachmentReference> colorReferences;
	colorReferences.push_back({ 0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 1, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });
	colorReferences.push_back({ 2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL });

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 3;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.pColorAttachments = colorReferences.data();
	subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
	subpass.pDepthStencilAttachment = &depthReference;

	// Use subpass dependencies for attachment layput transitions
	std::array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;


	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.pAttachments = attachmentDescs.data();
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachmentDescs.size());
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 2;
	renderPassInfo.pDependencies = dependencies.data();

	VK_CHECK_RESULT(vkCreateRenderPass(m_pWRenderer->m_SwapChain.device, &renderPassInfo, nullptr, &offScreenFrameBuf.renderPass));

	std::array<VkImageView, 4> attachments;
	attachments[0] = offScreenFrameBuf.position.view;
	attachments[1] = offScreenFrameBuf.nm.view;
	attachments[2] = offScreenFrameBuf.ssao.view;
	attachments[3] = offScreenFrameBuf.depth.view;

	VkFramebufferCreateInfo fbufCreateInfo = {};
	fbufCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	fbufCreateInfo.pNext = NULL;
	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	fbufCreateInfo.pAttachments = attachments.data();
	fbufCreateInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	fbufCreateInfo.width = offScreenFrameBuf.width;
	fbufCreateInfo.height = offScreenFrameBuf.height;
	fbufCreateInfo.layers = 1;
	VK_CHECK_RESULT(vkCreateFramebuffer(m_pWRenderer->m_SwapChain.device, &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer));

	// Create sampler to sample from the color attachments
	VkSamplerCreateInfo sampler = VkTools::Initializer::SamplerCreateInfo();
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 0;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(m_pWRenderer->m_SwapChain.device, &sampler, nullptr, &colorSampler));
}


void Renderer::ChangeLodBias(float delta)
{


}


void Renderer::BeginTextUpdate()
{

}


void Renderer::BuildCommandBuffers()
{
	VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(m_pWRenderer->m_CmdPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, 1);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &cmdGui));

	//Prepare FrameBuffer Commands
	PrepareFramebufferCommands();

	//Prepare SSAO Commands
	PrepareSSAOCommands();

	//Main Rendere Commands
	PrepareMainRendererCommands();
}

void Renderer::UpdateUniformBuffers()
{
	// Update matrices
	//m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, zNear, zFar);
	//m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 5.0f, g_zoom) + g_CameraPos);

	//m_uboVS.modelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.25f, 0.0f));
	//m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	//m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	//m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));


	//m_uboVS.normal = glm::inverseTranspose(m_uboVS.modelMatrix);
	//m_uboVS.viewPos = glm::vec4(0.0f, 0.0f, -15.0f, 0.0f);

	m_Camera.updateAspectRatio((float)m_WindowWidth / (float)m_WindowHeight);
	m_uboVS.projectionMatrix = m_Camera.matrices.perspective;
	m_uboVS.viewMatrix = m_Camera.matrices.view;
	m_uboVS.modelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, g_zoom, 0.0f));
	m_uboVS.modelMatrix = glm::scale(m_uboVS.modelMatrix, glm::vec3(0.2, 0.2, 0.2));
	{
		// Map uniform buffer and update it
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.mesh.memory, 0, sizeof(m_uboVS), 0, (void **)&pData));
		memcpy(pData, &m_uboVS, sizeof(m_uboVS));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.mesh.memory);
	}

	//FullScreen
	uboFullScreen.projection = glm::ortho(0.0f, 1.0f, 0.0f, 1.0f, -1.0f, 1.0f);
	uboFullScreen.model = glm::mat4();
	{
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.vsFullScreen.memory, 0, sizeof(uboFullScreen), 0, (void **)&pData));
		memcpy(pData, &uboFullScreen, sizeof(uboFullScreen));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.vsFullScreen.memory);
	}
}

void Renderer::PrepareUniformBuffers()
{
	//Mesh uniform buffer
	CreateUniformBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(m_uboVS),
		uniformData.mesh,
		&m_uboVS);


	//prepare quad
	CreateUniformBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(quadUniformData),
		uniformData.quad,
		&quadUniformData);

	//prepare light
	CreateUniformBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(uboFragmentLights),
		uniformData.fsLights,
		&quadUniformData);

	//prepare fullscreen
	CreateUniformBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(uboFragmentLights),
		uniformData.vsFullScreen,
		&uboFullScreen);

	//ssao
	CreateUniformBuffer(
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		sizeof(uboFragmentLights),
		uniformData.ssao,
		&uboSSAO);


	// Init some values
	m_uboVS.instancePos[0] = glm::vec4(0.0f);
	m_uboVS.instancePos[1] = glm::vec4(-4.0f, 0.0, -4.0f, 0.0f);
	m_uboVS.instancePos[2] = glm::vec4(4.0f, 0.0, -4.0f, 0.0f);

	InitializeSSAOData();
	UpdateUniformBuffers();
	UpdateQuadUniformData();
	UpdateUniformBuffersLights();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	//G-Buffer
	CreateFrameBuffer();

	//Debug Quad
	GenerateQuad();


	staticModel.InitFromFile("Geometry/nanosuit/nanosuit2.obj", GetAssetPath());

	std::vector<VkBufferObject_s> listStagingBuffers;
	listLocalBuffers.resize(staticModel.surfaces.size());
	listStagingBuffers.resize(staticModel.surfaces.size());
	listDescriptros.resize(staticModel.surfaces.size());

	m_pWRenderer->m_DescriptorPool = VK_NULL_HANDLE;
	SetupDescriptorPool();
	VkDescriptorSetAllocateInfo allocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(m_pWRenderer->m_DescriptorPool, &descriptorSetLayout, 1);


	// Image descriptor for the color attachement
	VkDescriptorImageInfo GBufferPosition =
		VkTools::Initializer::DescriptorImageInfo(
			colorSampler,
			offScreenFrameBuf.position.view,
			VK_IMAGE_LAYOUT_GENERAL);

	//Normal, Diffuse and Specular packed texture
	VkDescriptorImageInfo GBufferNM =
		VkTools::Initializer::DescriptorImageInfo(
			colorSampler,
			offScreenFrameBuf.nm.view,
			VK_IMAGE_LAYOUT_GENERAL);

	//ssao
	VkDescriptorImageInfo ssaoImage =
		VkTools::Initializer::DescriptorImageInfo(
			colorSampler,
			offScreenFrameBuf.ssao.view,
			VK_IMAGE_LAYOUT_GENERAL);

	//Noise generated texture 
	VkDescriptorImageInfo noiseImage = VkTools::Initializer::DescriptorImageInfo
	(m_NoiseGeneratedTexture.sampler, m_NoiseGeneratedTexture.view, VK_IMAGE_LAYOUT_GENERAL);


	// Debug Descriptor
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &quadDescriptorSet));
	std::vector<VkWriteDescriptorSet> quadWriteDescriptorSets =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.quad.descriptor),

		// Binding 1: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, &GBufferPosition),

		// Binding 2: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2, &GBufferNM),

		//Binding 3: SSAO image
		VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3, &ssaoImage)

	};
	vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, quadWriteDescriptorSets.size(), quadWriteDescriptorSets.data(), 0, NULL);

	// FullScreen Descriptor
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &defferedModelDescriptorSet));
	std::vector<VkWriteDescriptorSet> defferedWriteModelDescriptorSet =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::WriteDescriptorSet(defferedModelDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.vsFullScreen.descriptor),

		// Binding 1: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(defferedModelDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, &GBufferPosition),

		// Binding 2: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(defferedModelDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2, &GBufferNM),

		//Binding 4
		VkTools::Initializer::WriteDescriptorSet(defferedModelDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 4, &uniformData.fsLights.descriptor),
	};
	vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, defferedWriteModelDescriptorSet.size(), defferedWriteModelDescriptorSet.data(), 0, NULL);


	//SSAO descriptors
	VkDescriptorSetAllocateInfo ssaoallocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(m_pWRenderer->m_DescriptorPool, &ssaoDescriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &ssaoallocInfo, &ssaoDescriptorSet));
	std::vector<VkWriteDescriptorSet> ssaoWriteModelDescriptorSet =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::WriteDescriptorSet(ssaoDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.quad.descriptor),

		// Binding 0 : Fragment uniform buffer
		VkTools::Initializer::WriteDescriptorSet(ssaoDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0, &uniformData.ssao.descriptor),

		// Binding 1: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(ssaoDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, &GBufferPosition),

		// Binding 2: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(ssaoDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,2, &GBufferNM),

		// Binding 3: Image descriptor
		//VkTools::Initializer::WriteDescriptorSet(defferedModelDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,3, &noiseImage),

		//Binding 4: Image descriptor
		VkTools::Initializer::WriteDescriptorSet(ssaoDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4, &ssaoImage),
	};
	vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, ssaoWriteModelDescriptorSet.size(), ssaoWriteModelDescriptorSet.data(), 0, NULL);

	for (int i = 0; i < staticModel.surfaces.size(); i++)
	{
		//Get triangles
		srfTriangles_t* tr = staticModel.surfaces[i].geometry;


		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &listDescriptros[i]));
		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			//uniform descriptor
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0,&uniformData.mesh.descriptor)
		};

		//We need this because descriptorset will take pointer to vkDescriptorImageInfo.
		//So if we delete it before update descriptor sets. The data will not be sented to gpu. Will pointing to memory address in which nothing exist anymore
		//The program won't break. But you will see problems in fragment shader
		std::vector<VkDescriptorImageInfo> descriptors;
		descriptors.reserve(staticModel.surfaces[i].numMaterials);

		//Get all materials
		for (uint32_t j = 0; j < staticModel.surfaces[i].numMaterials; j++)
		{
			VkTools::VulkanTexture* pTexture = staticModel.surfaces[i].material[j].getTexture();
			descriptors.push_back(VkTools::Initializer::DescriptorImageInfo(pTexture->sampler, pTexture->view, VK_IMAGE_LAYOUT_GENERAL));
			writeDescriptorSets.push_back(VkTools::Initializer::WriteDescriptorSet(listDescriptros[i], VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, j + 1, &descriptors[j]));
		}
		//Update descriptor set
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);


		//Create stagging buffer first
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			static_cast<uint32_t>(sizeof(drawVert) * tr->numVerts),
			listStagingBuffers[i],
			tr->verts);

		//Create Local Copy
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			static_cast<uint32_t>(sizeof(drawVert) * tr->numVerts),
			listLocalBuffers[i]);


		//Create new command buffer
		VkCommandBuffer copyCmd = GetCommandBuffer(true);


		//Submit info to the queue
		VkBufferObject::SubmitBufferObjects(
			copyCmd,
			m_pWRenderer->m_Queue,
			*m_pWRenderer,
			static_cast<uint32_t>(sizeof(drawVert) * tr->numVerts),
			listStagingBuffers[i],
			listLocalBuffers[i], (drawVertFlags::Vertex | drawVertFlags::Normal | drawVertFlags::Uv | drawVertFlags::Tangent | drawVertFlags::Binormal));

		meshSize.push_back(tr->numVerts);

		numVerts += tr->numVerts;
		numUvs += tr->numVerts;
		numNormals += tr->numVerts;
	}
}


void Renderer::PreparePipeline()
{
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState =
		VkTools::Initializer::PipelineInputAssemblyStateCreateInfo(
			VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			0,
			VK_FALSE);

	VkPipelineRasterizationStateCreateInfo rasterizationState =
		VkTools::Initializer::PipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_CLOCKWISE,
			0);

	VkPipelineColorBlendAttachmentState blendAttachmentState =
		VkTools::Initializer::PipelineColorBlendAttachmentState(
			0xf,
			VK_FALSE);

	VkPipelineColorBlendStateCreateInfo colorBlendState =
		VkTools::Initializer::PipelineColorBlendStateCreateInfo(
			1,
			&blendAttachmentState);

	VkPipelineDepthStencilStateCreateInfo depthStencilState =
		VkTools::Initializer::PipelineDepthStencilStateCreateInfo(
			VK_TRUE,
			VK_TRUE,
			VK_COMPARE_OP_LESS_OR_EQUAL);

	VkPipelineViewportStateCreateInfo viewportState =
		VkTools::Initializer::PipelineViewportStateCreateInfo(1, 1, 0);

	VkPipelineMultisampleStateCreateInfo multisampleState =
		VkTools::Initializer::PipelineMultisampleStateCreateInfo(
			VK_SAMPLE_COUNT_1_BIT,
			0);

	std::vector<VkDynamicState> dynamicStateEnables = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState =
		VkTools::Initializer::PipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			static_cast<uint32_t>(dynamicStateEnables.size()),
			0);

	// Load shaders
	//Shaders are loaded from the SPIR-V format, which can be generated from glsl
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/SSAO/DefferedModel.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/SSAO/DefferedModel.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


	VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		VkTools::Initializer::PipelineCreateInfo(
			pipelineLayout,
			m_pWRenderer->m_RenderPass,
			0);

	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &quadMesh.vertex.inputState;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyState;
	pipelineCreateInfo.pRasterizationState = &rasterizationState;
	pipelineCreateInfo.pColorBlendState = &colorBlendState;
	pipelineCreateInfo.pMultisampleState = &multisampleState;
	pipelineCreateInfo.pViewportState = &viewportState;
	pipelineCreateInfo.pDepthStencilState = &depthStencilState;
	pipelineCreateInfo.renderPass = m_pWRenderer->m_RenderPass;
	pipelineCreateInfo.pDynamicState = &dynamicState;

	// Create rendering pipeline
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &pipeline));

	//Debug Quad Pipeline
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/SSAO/DebugQuad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/SSAO/DebugQuad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineCreateInfo.pVertexInputState = &quadMesh.vertex.inputState;

	//Turn off culling
	rasterizationState =
		VkTools::Initializer::PipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_NONE,
			VK_FRONT_FACE_CLOCKWISE,
			0);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &quadPipeline));


	//SSAO Pipeline
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/SSAO/SSAO.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/SSAO/SSAO.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	pipelineCreateInfo.pVertexInputState = nullptr;
	pipelineCreateInfo.layout = ssaoPipelineLayout;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &ssaoPipeline));


	//G-Buffer
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/SSAO/DefferedMRT.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/SSAO/DefferedMRT.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Separate render pass
	pipelineCreateInfo.renderPass = offScreenFrameBuf.renderPass;

	// Separate layout
	pipelineCreateInfo.layout = frameBufferPipelineLayout;

	// Blend attachment states required for all color attachments
	// This is important, as color write mask will otherwise be 0x0 and you
	// won't see anything rendered to the attachment
	std::array<VkPipelineColorBlendAttachmentState, 3> blendAttachmentStates =
	{
		VkTools::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		VkTools::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
		VkTools::Initializer::PipelineColorBlendAttachmentState(0xf, VK_FALSE),
	};
	colorBlendState.attachmentCount = static_cast<uint32_t>(blendAttachmentStates.size());
	colorBlendState.pAttachments = blendAttachmentStates.data();

	pipelineCreateInfo.pVertexInputState = &listLocalBuffers[0].inputState;

	//Turn on culling again
	rasterizationState =
		VkTools::Initializer::PipelineRasterizationStateCreateInfo(
			VK_POLYGON_MODE_FILL,
			VK_CULL_MODE_BACK_BIT,
			VK_FRONT_FACE_CLOCKWISE,
			0);

	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &frameBufferPipeline));
}



void Renderer::VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling, bool bUseCubeMap)
{

}


void Renderer::SetupDescriptorSetLayout()
{
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0),

			// Binding 1 : Fragment shader image sampler
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1),

			// Binding 2 : Fragment shader image sampler
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2),

			// Binding 3 : Fragment shader image sampler
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3),

			// Binding 4 : Fragment shader uniform buffer
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,4),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, &descriptorLayout, nullptr, &descriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));


		VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &frameBufferPipelineLayout));
	}

	//SSAO
	{
		std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0),

			// Binding 0 : Fragment Uniform
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT, 0),

			// Binding 1 : Fragment shader image sampler
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1),

			// Binding 2 : Fragment shader image sampler
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2),

			// Binding 3 : Fragment shader uniform buffer
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3),

			// Binding 4 : Fragment shader uniform buffer
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,4),
		};

		VkDescriptorSetLayoutCreateInfo descriptorLayout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
		VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, &descriptorLayout, nullptr, &ssaoDescriptorSetLayout));

		VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
		VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &ssaoPipelineLayout));
	}
}

void Renderer::SetupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 23),
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8 * 4 + 4)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(poolSizes.size(), poolSizes.data(), 10);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_pWRenderer->m_SwapChain.device, &descriptorPoolInfo, nullptr, &m_pWRenderer->m_DescriptorPool));
}


void Renderer::SetupDescriptorSet()
{

}

void Renderer::StartFrame()
{

	{
		// Get next image in the swap chain (back/front buffer)
		VK_CHECK_RESULT(m_pWRenderer->m_SwapChain.GetNextImage(Semaphores.presentComplete, &m_pWRenderer->m_currentBuffer));

		// Submit the post present image barrier to transform the image back to a color attachment
		// that can be used to write to by our render pass
		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pWRenderer->m_PostPresentCmdBuffers[m_pWRenderer->m_currentBuffer];

		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));

		// Make sure that the image barrier command submitted to the queue 
		// has finished executing
		VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));
	}



	{

		//Submit model
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();

		//Draw GBuffer
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &GBufferScreenCmdBuffer;


		// Wait for swap chain presentation to finish
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &Semaphores.presentComplete;


		//Signal ready for model render to complete
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &GBufferSemaphore;
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));

		//Draw SSAO
		//submitInfo.pWaitSemaphores = &GBufferSemaphore;
		//submitInfo.pSignalSemaphores = &Semaphores.ssaoSemaphore;
		//submitInfo.pCommandBuffers = &ssaoCmdBuffer;
		//VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));

		//Draw Model
		submitInfo.pWaitSemaphores = &GBufferSemaphore;
		submitInfo.pCommandBuffers = &m_pWRenderer->m_DrawCmdBuffers[m_pWRenderer->m_currentBuffer];
		submitInfo.pSignalSemaphores = &Semaphores.renderComplete;
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));


		//Wait for color output before rendering text
		submitInfo.pWaitDstStageMask = &stageFlags;

		//ImGUI Render
		ImGui_ImplGlfwVulkan_Render(cmdGui, m_pWRenderer->m_currentBuffer);

		// Wait model rendering to finnish
		submitInfo.pWaitSemaphores = &Semaphores.renderComplete;
		//Signal ready for text to completeS
		submitInfo.pSignalSemaphores = &Semaphores.textOverlayComplete;

		submitInfo.pCommandBuffers = &cmdGui;
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	}

	{
		// Submit pre present image barrier to transform the image from color attachment to present(khr) for presenting to the swap chain
		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pWRenderer->m_PrePresentCmdBuffers[m_pWRenderer->m_currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));


		// Present the current buffer to the swap chain
		// We pass the signal semaphore from the submit info
		// to ensure that the image is not rendered until
		// all commands have been submitted
		VK_CHECK_RESULT(m_pWRenderer->m_SwapChain.QueuePresent(m_pWRenderer->m_Queue, m_pWRenderer->m_currentBuffer, Semaphores.textOverlayComplete));
	}
}



void Renderer::LoadAssets()
{
	
}


//Main global Renderer
Renderer g_Renderer;

int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(720, 1280, false, HandleWindowMessages);
	ImGui_ImplGlfwVulkan_MouseOnWinodws(true); //for now will work. Should test against windows rect

#if defined(_WIN32)
	MSG msg;
#endif

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();
		ImGui_ImplGlfwVulkan_NewFrame(g_Renderer.frameTimer);
		g_Renderer.ImguiRender();

		//Update once camera is moved
		if (g_Renderer.m_bViewUpdated)
		{
			g_Renderer.m_bViewUpdated = false;
			g_Renderer.UpdateUniformBuffers();
			g_Renderer.UpdateQuadUniformData();
		}

		//Update lights all the time as they move
		g_Renderer.UpdateUniformBuffersLights();

		g_Renderer.StartFrame();
		g_Renderer.EndFrame(nullptr);
		auto tEnd = std::chrono::high_resolution_clock::now();

		g_Renderer.frameCounter++;
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		g_Renderer.frameTimer = (float)tDiff / 1000.0f;
		g_Renderer.m_Camera.update(g_Renderer.frameTimer);
		if (g_Renderer.m_Camera.moving())
		{
			g_Renderer.m_bViewUpdated = true;
		}

		g_Renderer.fpsTimer += (float)tDiff;
		if (g_Renderer.fpsTimer > 1000.0f)
		{
			g_Renderer.BeginTextUpdate();
			g_Renderer.lastFPS = g_Renderer.frameCounter;
			g_Renderer.fpsTimer = 0.0f;
			g_Renderer.frameCounter = 0;
		}
	}
	g_Renderer.VShutdown();
	return 0;
}


LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_CLOSE:
		//prepared = false;
		DestroyWindow(hWnd);
		PostQuitMessage(0);
		break;
	case WM_PAINT:
		//ValidateRect(window, NULL);
		//break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case KEY_P:
			//paused = !paused;
			break;
		case KEY_ESCAPE:
			PostQuitMessage(0);
			break;
		}

		if (g_Renderer.m_Camera.firstperson)
		{
			switch (wParam)
			{
			case KEY_W:
				g_Renderer.m_Camera.keys.up = true;
				break;
			case KEY_S:
				g_Renderer.m_Camera.keys.down = true;
				break;
			case KEY_A:
				g_Renderer.m_Camera.keys.left = true;
				break;
			case KEY_D:
				g_Renderer.m_Camera.keys.right = true;
				break;
			}
		}

		//keyPressed((uint32_t)wParam);
		break;
	case WM_KEYUP:
		if (g_Renderer.m_Camera.firstperson)
		{
			switch (wParam)
			{
			case KEY_W:
				g_Renderer.m_Camera.keys.up = false;
				break;
			case KEY_S:
				g_Renderer.m_Camera.keys.down = false;
				break;
			case KEY_A:
				g_Renderer.m_Camera.keys.left = false;
				break;
			case KEY_D:
				g_Renderer.m_Camera.keys.right = false;
				break;
			}
		}
		break;
	case WM_LBUTTONDOWN:
		g_MousePos.x = (float)LOWORD(lParam);
		g_MousePos.y = (float)HIWORD(lParam);
		ImGui_ImplGlfwVulkan_MousePressed(true);
		break;
	case WM_LBUTTONUP:
		ImGui_ImplGlfwVulkan_MousePressed(false);
		break;
	case WM_RBUTTONDOWN:
	case WM_MBUTTONDOWN:
		g_MousePos.x = (float)LOWORD(lParam);
		g_MousePos.y = (float)HIWORD(lParam);
		break;
	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_zoom += (float)wheelDelta * 0.005f * g_ZoomSpeed;
		g_Renderer.m_Camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * g_ZoomSpeed));
		g_Renderer.m_bViewUpdated = true;
		break;
	}
	case WM_MOUSEMOVE:
		ImGui_ImplGlfwVulkan_SetMousePos(LOWORD(lParam), HIWORD(lParam));

		if (wParam & MK_RBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_zoom += (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed;
			g_Renderer.m_Camera.translate(glm::vec3(-0.0f, 0.0f, (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			g_Renderer.m_bViewUpdated = true;
		}
		if (wParam & MK_LBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_Rotation.x += (g_MousePos.y - (float)posy) * 1.25f * g_RotationSpeed;
			g_Rotation.y -= (g_MousePos.x - (float)posx) * 1.25f * g_RotationSpeed;
			g_Renderer.m_Camera.rotate(glm::vec3((g_MousePos.y - (float)posy) * g_Renderer.m_Camera.rotationSpeed, -(g_MousePos.x - (float)posx) * g_Renderer.m_Camera.rotationSpeed, 0.0f));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			g_Renderer.m_bViewUpdated = true;
		}
		if (wParam & MK_MBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_Renderer.m_Camera.translate(glm::vec3(-(g_MousePos.x - (float)posx) * 0.01f, -(g_MousePos.y - (float)posy) * 0.01f, 0.0f));
			g_Renderer.m_bViewUpdated = true;

			g_MousePos.x = (float)posx;
			g_MousePos.y = (float)posy;
		}
		break;
	case WM_SIZE:
		RECT rect;
		if (GetWindowRect(hWnd, &rect))
		{
			WIN_Sizing(wParam, &rect);
		}
		break;
	case WM_EXITSIZEMOVE:
		if (g_bPrepared)
		{
			g_Renderer.VWindowResize(g_iDesktopHeight, g_iDesktopWidth);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


bool GenerateEvents(MSG& msg)
{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
	{
		if (msg.message == WM_QUIT)
		{
			return false;
		}
		else
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return true;
}


void WIN_Sizing(WORD side, RECT *rect)
{
	// restrict to a standard aspect ratio
	g_iDesktopWidth = rect->right - rect->left;
	g_iDesktopHeight = rect->bottom - rect->top;

	// Adjust width/height for window decoration
	RECT decoRect = { 0, 0, 0, 0 };
	AdjustWindowRect(&decoRect, WINDOW_STYLE | WS_SYSMENU, FALSE);
	uint32_t decoWidth = decoRect.right - decoRect.left;
	uint32_t decoHeight = decoRect.bottom - decoRect.top;

	g_iDesktopWidth -= decoWidth;
	g_iDesktopHeight -= decoHeight;
}