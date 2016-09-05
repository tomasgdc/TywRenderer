/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <RendererPch\stdafx.h>
#include <iomanip>

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


//Global variables
//====================================================================================

uint32_t	g_iDesktopWidth = 0;
uint32_t	g_iDesktopHeight = 0;
bool		g_bPrepared = false;

glm::vec3	g_Rotation = glm::vec3();
glm::vec3	g_CameraPos = glm::vec3();
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


//====================================================================================


//functions
//====================================================================================

bool GenerateEvents(MSG& msg);
void WIN_Sizing(WORD side, RECT *rect);
LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

//====================================================================================

// 16 bits of depth is enough for such a small scene
#define DEPTH_FORMAT VK_FORMAT_D16_UNORM

// Shadowmap properties
#if defined(__ANDROID__)
#define SHADOWMAP_DIM 1024
#else
#define SHADOWMAP_DIM 2048
#endif
#define SHADOWMAP_FILTER VK_FILTER_LINEAR

// Offscreen frame buffer properties
#define FB_COLOR_FORMAT VK_FORMAT_R8G8B8A8_UNORM

class Renderer final: public VKRenderer
{
	VkTools::VulkanTexture m_VkTexture;
	VkFont*	m_VkFont;
	glm::vec3 lightPos = glm::vec3();
	float lightFOV = 45.0f;

	// Keep depth range as small as possible
	// for better shadow map precision
	float zNear = 1.0f;
	float zFar = 96.0f;

	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 normal;
		glm::mat4 depthBiasMVP;
		glm::vec4 viewPos;
		float lodBias = 0.0f;
	}m_uboVS;


	struct {
		glm::mat4 depthMVP;
	} uboOffscreenVS;

	struct {
		glm::mat4 mvp;
	} quadUniformData;


	struct {
		glm::mat4 mvp;
		glm::mat4 modelMatrix;
		glm::mat4 shadowCoord;
	} testquadUniformData;


	struct {
		VkTools::UniformData offscreen;
		VkTools::UniformData quad;
		VkTools::UniformData testquad;
	}  uniformData;


	uint32_t numVerts = 0;
	uint32_t numUvs = 0;
	uint32_t numNormals = 0;

	std::vector<VkBufferObject_s>	listLocalBuffers;
	std::vector<VkDescriptorSet>	listDescriptros;
	std::vector<uint32_t>			meshSize;

	VkBufferObject_s				quadVbo;
	VkBufferObject_s				quadIndexVbo;
public:
	Renderer() {}
	~Renderer();

	void BuildCommandBuffers() override;

	void UpdateUniformBuffers() override;

	void PrepareUniformBuffers() override;

	void PrepareVertices(bool useStagingBuffers) override;

	void VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling) override;

	void PreparePipeline() override;


	void SetupDescriptorSet() override;

	void SetupDescriptorSetLayout() override;

	void SetupDescriptorPool() override;

	void LoadAssets() override;


	void StartFrame() override;


	void ChangeLodBias(float delta);
	void BeginTextUpdate();

	//Offscreen renderpass for depth pass
	void PrepareOffscreenRenderPass();
	void PrepareOffscreenFrameBuffer();
	void BuildOffscreenCommandBuffer();
	void UpdateOffscreenUniformBuffers();
	void UpdateLight();
	void GenerateQuad();
	void UpdateQuadUniformData();

	struct FrameBufferAttachment 
	{
		VkImage image;
		VkDeviceMemory mem;
		VkImageView view;
	};

	struct FrameBuffer 
	{
		int32_t width, height;
		VkFramebuffer frameBuffer;
		FrameBufferAttachment color, depth;
		VkRenderPass renderPass;
		VkSampler depthSampler;
	} offScreenFrameBuf;

	VkPipeline offscreenPipeline;
	VkPipelineLayout offscreenPipelineLayout;
	VkDescriptorSet offscreenDescriptorSet;

	VkCommandBuffer offScreenCmdBuffer = VK_NULL_HANDLE;
	// Semaphore used to synchronize offscreen rendering before using it's texture target for sampling
	VkSemaphore offscreenSemaphore = VK_NULL_HANDLE;

	// Depth bias (and slope) are used to avoid shadowing artefacts
	// Constant depth bias factor (always applied)
	float depthBiasConstant = 1.25f;
	// Slope depth bias factor, applied depending on polygon's slope
	float depthBiasSlope = 1.75f;


	//quad
	VkPipeline quadPipeline;
	VkPipeline testquadPipeline;
	VkPipelineLayout quadPipelineLayout;
	VkDescriptorSet  quadDescriptorSet;
	VkDescriptorSet  testquadDescriptorSet;
};


Renderer::~Renderer()
{
	SAFE_DELETE(m_VkFont);
	// Clean up used Vulkan resources 
	// Note : Inherited destructor cleans up resources stored in base class

	// Frame buffer
	vkDestroySampler(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depthSampler, nullptr);

	// Color attachment
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.color.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.color.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.color.mem, nullptr);

	// Depth attachment
	vkDestroyImageView(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.view, nullptr);
	vkDestroyImage(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.image, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.mem, nullptr);

	vkDestroyFramebuffer(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.frameBuffer, nullptr);

	vkDestroyRenderPass(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.renderPass, nullptr);

	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, offscreenPipeline, nullptr);
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, quadPipeline, nullptr);
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, testquadPipeline, nullptr);

	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, offscreenPipelineLayout, nullptr);
	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, quadPipelineLayout, nullptr);
	


	//vkDestroyDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, quaddes, nullptr);

	// Meshes


	// Uniform buffers
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.offscreen);
	VkTools::DestroyUniformData(m_pWRenderer->m_SwapChain.device, &uniformData.quad);

	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &offScreenCmdBuffer);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, offscreenSemaphore, nullptr);
}

void Renderer::UpdateQuadUniformData()
{
	static glm::mat4 perspective,  modelMatrix;
	float AR = (float)g_iDesktopHeight / (float)g_iDesktopWidth;

	perspective = glm::ortho(2.5f / AR, 0.0f, 0.0f, 2.5f, -1.0f, 1.0f);
	modelMatrix = glm::scale(glm::mat4(), glm::vec3(0.3f, 0.3f, 0.3f));
	modelMatrix = glm::translate(modelMatrix, glm::vec3(1, 1, 0));
	quadUniformData.mvp = perspective  * modelMatrix;

	// Map uniform buffer and update it
	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.quad.memory, 0, sizeof(quadUniformData), 0, (void **)&pData));
	memcpy(pData, &quadUniformData, sizeof(quadUniformData));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.quad.memory);
}


void Renderer::GenerateQuad()
{

#define DIM 1.0f
#define NORMAL { 0.0f, 0.0f, 1.0f }
	std::vector<drawVert> vertData =
	{
		{ glm::vec3(DIM,DIM,    0),	   glm::vec3(0,0,1),  glm::vec3(0,0,0), glm::vec3(0,0,0),  glm::vec2(1,1) },
		{ glm::vec3(-DIM,DIM,  0),     glm::vec3(0,0,1),  glm::vec3(0,0,0), glm::vec3(0,0,0),  glm::vec2(0,1) },
		{ glm::vec3(-DIM, -DIM,  0),   glm::vec3(0,0,1),  glm::vec3(0,0,0), glm::vec3(0,0,0),  glm::vec2(0,0) },
		{ glm::vec3(DIM,-DIM,  0),     glm::vec3(0,0,1),  glm::vec3(0,0,0), glm::vec3(0,0,0),  glm::vec2(1,0) },
	};

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2,3,0 };


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
		quadVbo);

	//Index
	VkBufferObject::CreateBuffer(
		m_pWRenderer->m_SwapChain,
		m_pWRenderer->m_DeviceMemoryProperties,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		static_cast<uint32_t>(sizeof(uint32_t) * indexBuffer.size()),
		quadIndexVbo);


	//Create new command buffer
	VkCommandBuffer copyCmd = GetCommandBuffer(true);


	//Submit info to the queue
	VkBufferObject::SubmitBufferObjects(
		copyCmd,
		m_pWRenderer->m_Queue,
		*m_pWRenderer,
		static_cast<uint32_t>(sizeof(drawVert) * vertData.size()),
		staggingBufferVbo,
		quadVbo, (drawVertFlags::Vertex | drawVertFlags::Uv | drawVertFlags::Normal));


	copyCmd = GetCommandBuffer(true);
	VkBufferObject::SubmitBufferObjects(
		copyCmd,
		m_pWRenderer->m_Queue,
		*m_pWRenderer,
		static_cast<uint32_t>(sizeof(uint32_t) * indexBuffer.size()),
		staggingIndexBufferVbo,
		quadIndexVbo, drawVertFlags::None);
}



void Renderer::UpdateLight()
{
	// Animate the light source
	lightPos.x = cos(glm::radians(timer * 360.0f));
	lightPos.y = -10.0f + sin(glm::radians(timer * 360.0f)) * 10.0f ;
	lightPos.z = 25.0f + sin(glm::radians(timer * 360.0f)) *20.0f;
}


void Renderer::PrepareOffscreenRenderPass()
{
	VkAttachmentDescription attDesc[2];
	attDesc[0].format = FB_COLOR_FORMAT;
	attDesc[0].samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// We only need depth information for shadow mapping
	// So we don't need to store the color information
	// after the render pass has finished
	attDesc[0].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attDesc[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attDesc[0].flags = VK_FLAGS_NONE;

	attDesc[1].format = DEPTH_FORMAT;
	attDesc[1].samples = VK_SAMPLE_COUNT_1_BIT;
	attDesc[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	// Since we need to copy the depth attachment contents to our texture
	// used for shadow mapping we must use STORE_OP_STORE to make sure that
	// the depth attachment contents are preserved after rendering to it 
	// has finished
	attDesc[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attDesc[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attDesc[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attDesc[1].initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attDesc[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	attDesc[1].flags = VK_FLAGS_NONE;

	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorReference;
	subpass.pDepthStencilAttachment = &depthReference;

	VkRenderPassCreateInfo renderPassCreateInfo = VkTools::Initializer::RenderPassCreateInfo();
	renderPassCreateInfo.attachmentCount = 2;
	renderPassCreateInfo.pAttachments = attDesc;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpass;

	VK_CHECK_RESULT(vkCreateRenderPass(m_pWRenderer->m_SwapChain.device, &renderPassCreateInfo, nullptr, &offScreenFrameBuf.renderPass));
}


// Setup the offscreen framebuffer for rendering the scene from
// light's point-of-view to
// The depth attachment of this framebuffer will then be used
// to sample frame in the fragment shader of the shadowing pass
void Renderer::PrepareOffscreenFrameBuffer()
{
	offScreenFrameBuf.width = SHADOWMAP_DIM;
	offScreenFrameBuf.height = SHADOWMAP_DIM;

	VkFormat fbColorFormat = FB_COLOR_FORMAT;

	// Color attachment
	VkImageCreateInfo image = VkTools::Initializer::ImageCreateInfo();
	image.imageType = VK_IMAGE_TYPE_2D;
	image.format = fbColorFormat;
	image.extent.width = offScreenFrameBuf.width;
	image.extent.height = offScreenFrameBuf.height;
	image.extent.depth = 1;
	image.mipLevels = 1;
	image.arrayLayers = 1;
	image.samples = VK_SAMPLE_COUNT_1_BIT;
	image.tiling = VK_IMAGE_TILING_OPTIMAL;
	// Image of the framebuffer is blit source
	image.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	VkMemoryAllocateInfo memAlloc = VkTools::Initializer::MemoryAllocateInfo();
	VkMemoryRequirements memReqs;

	VkImageViewCreateInfo colorImageView = VkTools::Initializer::ImageViewCreateInfo();
	colorImageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	colorImageView.format = fbColorFormat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = 1;
	VK_CHECK_RESULT(vkCreateImage(m_pWRenderer->m_SwapChain.device, &image, nullptr, &offScreenFrameBuf.color.image));

	vkGetImageMemoryRequirements(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.color.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &offScreenFrameBuf.color.mem));
	VK_CHECK_RESULT(vkBindImageMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.color.image, offScreenFrameBuf.color.mem, 0));


	//Create command buffer
	VkCommandBuffer layoutCmd;
	VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			VkTools::Initializer::CommandBufferAllocateInfo(
				m_pWRenderer->m_CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &layoutCmd));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
	VK_CHECK_RESULT(vkBeginCommandBuffer(layoutCmd, &cmdBufInfo));
	//Command buffer created and started


	VkTools::SetImageLayout(
		layoutCmd,
		offScreenFrameBuf.color.image,
		VK_IMAGE_ASPECT_COLOR_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

	colorImageView.image = offScreenFrameBuf.color.image;
	VK_CHECK_RESULT(vkCreateImageView(m_pWRenderer->m_SwapChain.device, &colorImageView, nullptr, &offScreenFrameBuf.color.view));

	// Depth stencil attachment
	image.format = DEPTH_FORMAT;
	// We will sample directly from the depth attachment for the shadow mapping
	image.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

	VkImageViewCreateInfo depthStencilView = VkTools::Initializer::ImageViewCreateInfo();
	depthStencilView.viewType = VK_IMAGE_VIEW_TYPE_2D;
	depthStencilView.format = DEPTH_FORMAT;
	depthStencilView.subresourceRange = {};
	depthStencilView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
	depthStencilView.subresourceRange.baseMipLevel = 0;
	depthStencilView.subresourceRange.levelCount = 1;
	depthStencilView.subresourceRange.baseArrayLayer = 0;
	depthStencilView.subresourceRange.layerCount = 1;
	VK_CHECK_RESULT(vkCreateImage(m_pWRenderer->m_SwapChain.device, &image, nullptr, &offScreenFrameBuf.depth.image));

	vkGetImageMemoryRequirements(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.image, &memReqs);
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_pWRenderer->m_DeviceMemoryProperties);
	VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &memAlloc, nullptr, &offScreenFrameBuf.depth.mem));
	VK_CHECK_RESULT(vkBindImageMemory(m_pWRenderer->m_SwapChain.device, offScreenFrameBuf.depth.image, offScreenFrameBuf.depth.mem, 0));

	// Set the initial layout to shader read instead of attachment 
	// This is done as the render loop does the actualy image layout transitions
	VkTools::SetImageLayout(
		layoutCmd,
		offScreenFrameBuf.depth.image,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);


	//Flush command buffer start
	VK_CHECK_RESULT(vkEndCommandBuffer(layoutCmd));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &layoutCmd;

	VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));
	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &layoutCmd);
	//Flush Command Buffer End


	depthStencilView.image = offScreenFrameBuf.depth.image;
	VK_CHECK_RESULT(vkCreateImageView(m_pWRenderer->m_SwapChain.device, &depthStencilView, nullptr, &offScreenFrameBuf.depth.view));

	// Create sampler to sample from to depth attachment 
	// Used to sample in the fragment shader for shadowed rendering
	VkSamplerCreateInfo sampler = VkTools::Initializer::SamplerCreateInfo();
	sampler.magFilter = SHADOWMAP_FILTER;
	sampler.minFilter = SHADOWMAP_FILTER;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.maxAnisotropy = 0;
	sampler.minLod = 0.0f;
	sampler.maxLod = 1.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	VK_CHECK_RESULT(vkCreateSampler(m_pWRenderer->m_SwapChain.device, &sampler, nullptr, &offScreenFrameBuf.depthSampler));

	VkImageView attachments[2];
	attachments[0] = offScreenFrameBuf.color.view;
	attachments[1] = offScreenFrameBuf.depth.view;

	PrepareOffscreenRenderPass();

	// Create frame buffer
	VkFramebufferCreateInfo fbufCreateInfo = VkTools::Initializer::FramebufferCreateInfo();
	fbufCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	fbufCreateInfo.attachmentCount = 2;
	fbufCreateInfo.pAttachments = attachments;
	fbufCreateInfo.width = offScreenFrameBuf.width;
	fbufCreateInfo.height = offScreenFrameBuf.height;
	fbufCreateInfo.layers = 1;

	VK_CHECK_RESULT(vkCreateFramebuffer(m_pWRenderer->m_SwapChain.device, &fbufCreateInfo, nullptr, &offScreenFrameBuf.frameBuffer));
}

void Renderer::BuildOffscreenCommandBuffer()
{
	if (offScreenCmdBuffer == VK_NULL_HANDLE)
	{
		VkCommandBufferAllocateInfo cmdBufAllocateInfo =
			VkTools::Initializer::CommandBufferAllocateInfo(
				m_pWRenderer->m_CmdPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1);

		VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &offScreenCmdBuffer));
	}

	// Create a semaphore used to synchronize offscreen rendering and usage
	VkSemaphoreCreateInfo semaphoreCreateInfo = VkTools::Initializer::SemaphoreCreateInfo();
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &offscreenSemaphore));

	VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	VkRenderPassBeginInfo renderPassBeginInfo = VkTools::Initializer::RenderPassBeginInfo();
	renderPassBeginInfo.renderPass = offScreenFrameBuf.renderPass;
	renderPassBeginInfo.framebuffer = offScreenFrameBuf.frameBuffer;
	renderPassBeginInfo.renderArea.offset.x = 0;
	renderPassBeginInfo.renderArea.offset.y = 0;
	renderPassBeginInfo.renderArea.extent.width = offScreenFrameBuf.width;
	renderPassBeginInfo.renderArea.extent.height = offScreenFrameBuf.height;
	renderPassBeginInfo.clearValueCount = 2;
	renderPassBeginInfo.pClearValues = clearValues;

	VK_CHECK_RESULT(vkBeginCommandBuffer(offScreenCmdBuffer, &cmdBufInfo));

	// Change back layout of the depth attachment after sampling in the fragment shader
	VkTools::SetImageLayout(
		offScreenCmdBuffer,
		offScreenFrameBuf.depth.image,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);

	VkViewport viewport = VkTools::Initializer::Viewport((float)offScreenFrameBuf.width, (float)offScreenFrameBuf.height, 0.0f, 1.0f);
	vkCmdSetViewport(offScreenCmdBuffer, 0, 1, &viewport);

	VkRect2D scissor = VkTools::Initializer::Rect2D(offScreenFrameBuf.width, offScreenFrameBuf.height, 0, 0);
	vkCmdSetScissor(offScreenCmdBuffer, 0, 1, &scissor);

	// Set depth bias (aka "Polygon offset")
	// Required to avoid shadow mapping artefacts
	vkCmdSetDepthBias(
		offScreenCmdBuffer,
		depthBiasConstant,
		0.0f,
		depthBiasSlope);

	vkCmdBeginRenderPass(offScreenCmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline);
	vkCmdBindDescriptorSets(offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipelineLayout, 0, 1, &offscreenDescriptorSet, 0, NULL);

	// Bind triangle vertex buffer (contains position and colors)
	VkDeviceSize offsets[1] = { 0 };

	for (int j = 0; j < listLocalBuffers.size(); j++)
	{
		// Bind descriptor sets describing shader binding points
		//vkCmdBindDescriptorSets(offScreenCmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &listDescriptros[j], 0, NULL);

		//Bind Buffer
		vkCmdBindVertexBuffers(offScreenCmdBuffer, VERTEX_BUFFER_BIND_ID, 1, &listLocalBuffers[j].buffer, offsets);

		//Draw
		vkCmdDraw(offScreenCmdBuffer, meshSize[j], 1, 0, 0);
	}

	vkCmdEndRenderPass(offScreenCmdBuffer);

	// Change layout of the depth attachment for sampling in the fragment shader
	VkTools::SetImageLayout(
		offScreenCmdBuffer,
		offScreenFrameBuf.depth.image,
		VK_IMAGE_ASPECT_DEPTH_BIT,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	VK_CHECK_RESULT(vkEndCommandBuffer(offScreenCmdBuffer));
}

void Renderer::UpdateOffscreenUniformBuffers()
{
	// Matrix from light's point of view
	//glm::mat4 depthProjectionMatrix = glm::perspective(glm::radians(lightFOV), 1.0f, zNear, zFar);
	//glm::mat4 depthViewMatrix = glm::lookAt(lightPos, glm::vec3(0.0f), glm::vec3(0, -1, 0));
	//glm::mat4 depthModelMatrix = glm::mat4();

	//uboOffscreenVS.depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;


	//Att the moment use models camera for light shadow
	uboOffscreenVS.depthMVP = m_uboVS.projectionMatrix * m_uboVS.viewMatrix * m_uboVS.modelMatrix;
	m_uboVS.depthBiasMVP = uboOffscreenVS.depthMVP;
	testquadUniformData.shadowCoord = uboOffscreenVS.depthMVP;

	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.offscreen.memory, 0, sizeof(uboOffscreenVS), 0, (void **)&pData));
	memcpy(pData, &uboOffscreenVS, sizeof(uboOffscreenVS));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.offscreen.memory);
}


void Renderer::ChangeLodBias(float delta)
{
	m_uboVS.lodBias += delta;
	if (m_uboVS.lodBias < 0.0f)
	{
		m_uboVS.lodBias = 0.0f;
	}
	if (m_uboVS.lodBias > m_VkTexture.mipLevels)
	{
		m_uboVS.lodBias = m_VkTexture.mipLevels;
	}

	UpdateUniformBuffers();
	UpdateQuadUniformData();
	m_VkFont->UpdateUniformBuffers(m_WindowWidth, m_WindowHeight, g_zoom);
}


void Renderer::BeginTextUpdate()
{
	m_VkFont->BeginTextUpdate();

	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << "ms-" << (frameTimer * 1000.0f) <<  "-fps-" << lastFPS;
	m_VkFont->AddText(-25, -30, 0.1, 0.1, ss.str());

	m_VkFont->EndTextUpdate();
}


void Renderer::BuildCommandBuffers()
{
	//Build Offscreen Command Buffers
	BuildOffscreenCommandBuffer();



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

		//shadow map display
		{
			vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipeline);
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipelineLayout, 0, 1, &quadDescriptorSet, 0, NULL);
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &quadVbo.buffer, offsets);
			vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], quadIndexVbo.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], 6, 1, 0, 0, 0);
		}

		//Test
		{
			vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, testquadPipeline);
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, quadPipelineLayout, 0, 1, &testquadDescriptorSet, 0, NULL);
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &quadVbo.buffer, offsets);
			vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], quadIndexVbo.buffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], 6, 1, 0, 0, 0);
		}

		//3D SCENE
		vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
		for (int j = 0; j < listLocalBuffers.size(); j++)
		{
			// Bind descriptor sets describing shader binding points
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &listDescriptros[j], 0, NULL);

			//Bind Buffer
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &listLocalBuffers[j].buffer, offsets);

			//Draw
			vkCmdDraw(m_pWRenderer->m_DrawCmdBuffers[i], meshSize[j], 1, 0, 0);
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

void Renderer::UpdateUniformBuffers()
{
	// Update matrices
	m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, zNear, zFar);

	m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 5.0f, g_zoom));

	m_uboVS.modelMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 5.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	m_uboVS.normal = glm::inverseTranspose(m_uboVS.modelMatrix);

	m_uboVS.viewPos = glm::vec4(0.0f, 0.0f, -15.0f, 0.0f);

	//m_uboVS.projectionMatrix = uboOffscreenVS.depthMVP;

	// Map uniform buffer and update it
	{
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(m_uboVS), 0, (void **)&pData));
		memcpy(pData, &m_uboVS, sizeof(m_uboVS));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory);
	}

	// test Map uniform buffer and update it
	testquadUniformData.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(50.0f, 50.0f, 50.0f));
	testquadUniformData.modelMatrix = glm::translate(testquadUniformData.modelMatrix, glm::vec3(0.0f, 0.0f, 0.2));


	testquadUniformData.mvp = m_uboVS.projectionMatrix * m_uboVS.viewMatrix  * m_uboVS.modelMatrix * testquadUniformData.modelMatrix;
	

	{
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformData.testquad.memory, 0, sizeof(testquadUniformData), 0, (void **)&pData));
		memcpy(pData, &testquadUniformData, sizeof(testquadUniformData));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformData.testquad.memory);
	}
}

void Renderer::PrepareUniformBuffers()
{

	//Create model uniform buffers
	{
		VkMemoryRequirements memReqs;

		// Vertex shader uniform buffer block
		VkBufferCreateInfo bufferInfo = {};
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(m_uboVS);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &uniformDataVS.buffer));
		// Get memory requirements including size, alignment and memory type 
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, uniformDataVS.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visibile memory access
		// Most implementations offer multiple memory tpyes and selecting the 
		// correct one to allocate memory from is important
		// We also want the buffer to be host coherent so we don't have to flush 
		// after every update. 
		// Note that this may affect performance so you might not want to do this 
		// in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		// Allocate memory for the uniform buffer
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(uniformDataVS.memory)));
		// Bind memory to buffer
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.buffer, uniformDataVS.memory, 0));

		// Store information in the uniform's descriptor
		uniformDataVS.descriptor.buffer = uniformDataVS.buffer;
		uniformDataVS.descriptor.offset = 0;
		uniformDataVS.descriptor.range = sizeof(m_uboVS);
	}


	//Prepare Offscreen buffers
	//Create model uniform buffers
	{
		VkMemoryRequirements memReqs;

		// Vertex shader uniform buffer block
		VkBufferCreateInfo bufferInfo = {};
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(uboOffscreenVS);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &uniformData.offscreen.buffer));
		// Get memory requirements including size, alignment and memory type 
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, uniformData.offscreen.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visibile memory access
		// Most implementations offer multiple memory tpyes and selecting the 
		// correct one to allocate memory from is important
		// We also want the buffer to be host coherent so we don't have to flush 
		// after every update. 
		// Note that this may affect performance so you might not want to do this 
		// in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		// Allocate memory for the uniform buffer
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(uniformData.offscreen.memory)));
		// Bind memory to buffer
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, uniformData.offscreen.buffer, uniformData.offscreen.memory, 0));

		// Store information in the uniform's descriptor
		uniformData.offscreen.descriptor.buffer = uniformData.offscreen.buffer;
		uniformData.offscreen.descriptor.offset = 0;
		uniformData.offscreen.descriptor.range = sizeof(uboOffscreenVS);
	}


	//prepare quad
	{
		VkMemoryRequirements memReqs;

		// Vertex shader uniform buffer block
		VkBufferCreateInfo bufferInfo = {};
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(quadUniformData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &uniformData.quad.buffer));
		// Get memory requirements including size, alignment and memory type 
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, uniformData.quad.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visibile memory access
		// Most implementations offer multiple memory tpyes and selecting the 
		// correct one to allocate memory from is important
		// We also want the buffer to be host coherent so we don't have to flush 
		// after every update. 
		// Note that this may affect performance so you might not want to do this 
		// in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		// Allocate memory for the uniform buffer
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(uniformData.quad.memory)));
		// Bind memory to buffer
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, uniformData.quad.buffer, uniformData.quad.memory, 0));

		// Store information in the uniform's descriptor
		uniformData.quad.descriptor.buffer = uniformData.quad.buffer;
		uniformData.quad.descriptor.offset = 0;
		uniformData.quad.descriptor.range = sizeof(quadUniformData);
	}


	//prepare test quad
	{
		VkMemoryRequirements memReqs;

		// Vertex shader uniform buffer block
		VkBufferCreateInfo bufferInfo = {};
		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.pNext = NULL;
		allocInfo.allocationSize = 0;
		allocInfo.memoryTypeIndex = 0;

		bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferInfo.size = sizeof(testquadUniformData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &uniformData.testquad.buffer));
		// Get memory requirements including size, alignment and memory type 
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, uniformData.testquad.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		// Get the memory type index that supports host visibile memory access
		// Most implementations offer multiple memory tpyes and selecting the 
		// correct one to allocate memory from is important
		// We also want the buffer to be host coherent so we don't have to flush 
		// after every update. 
		// Note that this may affect performance so you might not want to do this 
		// in a real world application that updates buffers on a regular base
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		// Allocate memory for the uniform buffer
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(uniformData.testquad.memory)));
		// Bind memory to buffer
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, uniformData.testquad.buffer, uniformData.testquad.memory, 0));

		// Store information in the uniform's descriptor
		uniformData.testquad.descriptor.buffer = uniformData.testquad.buffer;
		uniformData.testquad.descriptor.offset = 0;
		uniformData.testquad.descriptor.range = sizeof(testquadUniformData);
	}
	UpdateLight();
	UpdateUniformBuffers();
	UpdateOffscreenUniformBuffers();
	UpdateQuadUniformData();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	//Create offscreen  frame buffer
	PrepareOffscreenFrameBuffer();

	//PrepareQUAD
	GenerateQuad();

	m_pWRenderer->m_DescriptorPool = VK_NULL_HANDLE;

	//Setup descriptor pool
	SetupDescriptorPool();

	
		// Textured quad descriptor set
		VkDescriptorSetAllocateInfo allocInfo =
			VkTools::Initializer::DescriptorSetAllocateInfo(
				m_pWRenderer->m_DescriptorPool,
				&descriptorSetLayout,
				1);

		// Image descriptor for the shadow map attachment
		VkDescriptorImageInfo offscreenDescriptor =
			VkTools::Initializer::DescriptorImageInfo(
				offScreenFrameBuf.depthSampler,
				offScreenFrameBuf.depth.view,
				VK_IMAGE_LAYOUT_GENERAL);

		// Offscreen
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &quadDescriptorSet));
		std::vector<VkWriteDescriptorSet> quadWriteDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.quad.descriptor),

			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(quadDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, &offscreenDescriptor)

		};
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, quadWriteDescriptorSets.size(), quadWriteDescriptorSets.data(), 0, NULL);
	
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &testquadDescriptorSet));
		std::vector<VkWriteDescriptorSet> testquadWriteDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(testquadDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.testquad.descriptor),

			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(testquadDescriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,1, &offscreenDescriptor)

		};
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, testquadWriteDescriptorSets.size(), testquadWriteDescriptorSets.data(), 0, NULL);
		

		// Offscreen
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &offscreenDescriptorSet));
		std::vector<VkWriteDescriptorSet> offscreenWriteDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(offscreenDescriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,0, &uniformData.offscreen.descriptor),
		};
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, offscreenWriteDescriptorSets.size(), offscreenWriteDescriptorSets.data(), 0, NULL);




	//Create font pipeline
	m_VkFont->CreateFontVk((GetAssetPath() + "Textures/freetype/AmazDooMLeft.ttf"), 64, 96);

	m_VkFont->SetupDescriptorPool();
	m_VkFont->SetupDescriptorSetLayout();
	m_VkFont->PrepareUniformBuffers();
	m_VkFont->InitializeChars("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-:.@1234567890", *m_pTextureLoader);
	m_VkFont->PrepareResources(g_iDesktopWidth, g_iDesktopHeight);


	BeginTextUpdate();


	RenderModelStatic staticModel;
	staticModel.InitFromFile("Geometry/nanosuit/nanosuit2.obj", GetAssetPath());


	//RenderModelAssimp assimpModel;
	//assimpModel.InitFromFile("Geometry/cyberwarrior/warrior.fxb", GetAssetPath());


	
	std::vector<VkBufferObject_s> listStagingBuffers;

	listLocalBuffers.resize(staticModel.surfaces.size());
	listStagingBuffers.resize(staticModel.surfaces.size());
	listDescriptros.resize(staticModel.surfaces.size());
	for (int i = 0; i < staticModel.surfaces.size(); i++) 
	{
		//Get triangles
		srfTriangles_t* tr = staticModel.surfaces[i].geometry;

		
		VkTools::VulkanTexture* vkBumpTexture = staticModel.surfaces[i].material[0].getTexture();
		VkTools::VulkanTexture* vkDiffuseTexture = staticModel.surfaces[i].material[1].getTexture();


		/*
			=================================================================================================================
			START SETUP DESCRIPTOR SET
			=================================================================================================================
		*/
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &listDescriptros[i]));

		// Image descriptor for the color map texture
		VkDescriptorImageInfo texDescriptorDiffuse = VkTools::Initializer::DescriptorImageInfo(vkDiffuseTexture->sampler, vkDiffuseTexture->view, VK_IMAGE_LAYOUT_GENERAL);
		VkDescriptorImageInfo texDescriptorNormal = VkTools::Initializer::DescriptorImageInfo(vkBumpTexture->sampler, vkBumpTexture->view, VK_IMAGE_LAYOUT_GENERAL);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0,&uniformDataVS.descriptor),

			// Binding 1 : Fragment shader texture sampler
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&texDescriptorDiffuse),

			// Binding 2 : Fragment shader texture sampler
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2,&texDescriptorNormal),

			// Binding 3 : Fragment shader texture sampler
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 3,&offscreenDescriptor)
		};
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

		/*
		=================================================================================================================
		END SETUP DESCRIPTOR SET
		=================================================================================================================
		*/



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

	// Create our rendering pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate
	// fixed states
	// This replaces OpenGL's huge (and cumbersome) state machine
	// A pipeline is then stored and hashed on the GPU making
	// pipeline changes much faster than having to set dozens of 
	// states
	// In a real world application you'd have dozens of pipelines
	// for every shader set used in a scene
	// Note that there are a few states that are not stored with
	// the pipeline. These are called dynamic states and the 
	// pipeline only stores that they are used with this pipeline,
	// but not their states

	// Assign states
	// Assign pipeline state create information
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};

	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline
	pipelineCreateInfo.layout = pipelineLayout;
	// Renderpass this pipeline is attached to
	pipelineCreateInfo.renderPass = m_pWRenderer->m_RenderPass;

	// Vertex input state
	// Describes the topoloy used with this pipeline
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {};
	inputAssemblyState.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	// This pipeline renders vertex data as triangle lists
	inputAssemblyState.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo rasterizationState = {};
	rasterizationState.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	// Solid polygon mode
	rasterizationState.polygonMode = VK_POLYGON_MODE_FILL;
	// No culling
	rasterizationState.cullMode = VK_CULL_MODE_NONE;
	rasterizationState.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationState.depthClampEnable = VK_FALSE;
	rasterizationState.rasterizerDiscardEnable = VK_FALSE;
	rasterizationState.depthBiasEnable = VK_FALSE;
	rasterizationState.lineWidth = 1.0f;

	// Color blend state
	// Describes blend modes and color masks
	VkPipelineColorBlendStateCreateInfo colorBlendState = {};
	colorBlendState.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	// One blend attachment state
	// Blending is not used in this example
	VkPipelineColorBlendAttachmentState blendAttachmentState[1] = {};
	blendAttachmentState[0].colorWriteMask = 0xf;
	blendAttachmentState[0].blendEnable = VK_FALSE;
	colorBlendState.attachmentCount = 1;
	colorBlendState.pAttachments = blendAttachmentState;

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportState = {};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	// One viewport
	viewportState.viewportCount = 1;
	// One scissor rectangle
	viewportState.scissorCount = 1;

	// Enable dynamic states
	// Describes the dynamic states to be used with this pipeline
	// Dynamic states can be set even after the pipeline has been created
	// So there is no need to create new pipelines just for changing
	// a viewport's dimensions or a scissor box
	VkPipelineDynamicStateCreateInfo dynamicState = {};
	// The dynamic state properties themselves are stored in the command buffer
	std::vector<VkDynamicState> dynamicStateEnables;
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pDynamicStates = dynamicStateEnables.data();
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());

	// Depth and stencil state
	// Describes depth and stenctil test and compare ops
	VkPipelineDepthStencilStateCreateInfo depthStencilState = {};
	// Basic depth compare setup with depth writes and depth test enabled
	// No stencil used 
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.back.failOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.passOp = VK_STENCIL_OP_KEEP;
	depthStencilState.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = depthStencilState.back;

	// Multi sampling state
	VkPipelineMultisampleStateCreateInfo multisampleState = {};
	multisampleState.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleState.pSampleMask = NULL;
	// No multi sampling used in this example
	multisampleState.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	// Load shaders
	//Shaders are loaded from the SPIR-V format, which can be generated from glsl
	std::array<VkPipelineShaderStageCreateInfo, 2> shaderStages;
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/shadowmapping.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/shadowmapping.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


	// Create Pipeline state VI-IA-VS-VP-RS-FS-CB
	pipelineCreateInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	pipelineCreateInfo.pStages = shaderStages.data();
	pipelineCreateInfo.pVertexInputState = &listLocalBuffers[0].inputState;
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


	// quad pipeline
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/quad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/quad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	pipelineCreateInfo.layout = quadPipelineLayout;
	pipelineCreateInfo.pVertexInputState = &quadVbo.inputState;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &quadPipeline));



	//test quad pipeline
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/testquad.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/testquad.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &testquadPipeline));


	// Offscreen pipeline
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/offscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/ShadowMapping/offscreen.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	// Cull front faces
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	// Enable depth bias
	rasterizationState.depthBiasEnable = VK_TRUE;
	// Add depth bias to dynamic state, so we can change it at runtime
	dynamicStateEnables.push_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
	dynamicState =
		VkTools::Initializer::PipelineDynamicStateCreateInfo(
			dynamicStateEnables.data(),
			dynamicStateEnables.size(),
			0);

	pipelineCreateInfo.layout = offscreenPipelineLayout;
	pipelineCreateInfo.renderPass = offScreenFrameBuf.renderPass;
	VK_CHECK_RESULT(vkCreateGraphicsPipelines(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_PipelineCache, 1, &pipelineCreateInfo, nullptr, &offscreenPipeline));
}



void Renderer::VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling)
{
	//m_pTextureLoader->LoadTexture(fileName, format, &m_VkTexture);
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
			VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,3)
		};
		{
			VkDescriptorSetLayoutCreateInfo descriptorLayout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
			VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, &descriptorLayout, nullptr, &descriptorSetLayout));

			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
			VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
		}

		{
			VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);

			// Offscreen pipeline layout
			VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &offscreenPipelineLayout));

			//quad
			VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &quadPipelineLayout));
		}
	}
}

void Renderer::SetupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, (9*2)),
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (10*3))
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(poolSizes.size(),poolSizes.data(),7*2);
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


		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		
		{
			submitInfo.commandBufferCount = 1;
			submitInfo.pCommandBuffers = &offScreenCmdBuffer;

			// Wait for swap chain presentation to finish
			submitInfo.waitSemaphoreCount = 1;
			submitInfo.pWaitSemaphores = &Semaphores.presentComplete;

			//Signal ready for model render to complete
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.pSignalSemaphores = &offscreenSemaphore;
			VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
		}

		{
			// Wait offscreen rendering to finnish
			submitInfo.pWaitSemaphores = &offscreenSemaphore;

			//Signal ready for model rendering
			submitInfo.pSignalSemaphores = &Semaphores.renderComplete;

			submitInfo.pCommandBuffers = &m_pWRenderer->m_DrawCmdBuffers[m_pWRenderer->m_currentBuffer];
			VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
		}


		{
			//Wait for color output before rendering text
			submitInfo.pWaitDstStageMask = &stageFlags;

			// Wait model rendering to finnish
			submitInfo.pWaitSemaphores = &Semaphores.renderComplete;
			//Signal ready for text to completeS
			submitInfo.pSignalSemaphores = &Semaphores.textOverlayComplete;

			submitInfo.pCommandBuffers = &m_VkFont->cmdBuffers[m_pWRenderer->m_currentBuffer];
			VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
		}


		// Reset stage mask
		submitInfo.pWaitDstStageMask = &submitPipelineStages;
		// Reset wait and signal semaphores for rendering next frame
		// Wait for swap chain presentation to finish
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &Semaphores.presentComplete;
		// Signal ready with offscreen semaphore
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &Semaphores.renderComplete;

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
	m_VkFont = TYW_NEW VkFont(m_pWRenderer->m_SwapChain.physicalDevice, m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_Queue, m_pWRenderer->m_FrameBuffers, m_pWRenderer->m_SwapChain.colorFormat, m_pWRenderer->m_SwapChain.depthFormat, &m_WindowWidth, &m_WindowHeight);
}


//Main global Renderer
Renderer g_Renderer;

int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(800, 600, false, HandleWindowMessages);
	
#if defined(_WIN32)
	MSG msg;
#endif

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();

		g_Renderer.UpdateLight();
		g_Renderer.UpdateOffscreenUniformBuffers();

		g_Renderer.StartFrame();
		g_Renderer.EndFrame(nullptr);
		auto tEnd = std::chrono::high_resolution_clock::now();
		
		g_Renderer.frameCounter++;
		auto tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		g_Renderer.frameTimer = (float)tDiff / 1000.0f;
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
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 0x50:
			//paused = !paused;
			break;
		case VK_F1:
			//if (enableTextOverlay)
		{
			//textOverlay->visible = !textOverlay->visible;
		}
		break;
		case VK_ESCAPE:
			PostQuitMessage(0);
			break;
		}



		switch ((uint32_t)wParam)
		{
		case 0x6B:
		case GAMEPAD_BUTTON_R1:
			g_Renderer.ChangeLodBias(0.1f);
			break;
		case 0x6D:
		case GAMEPAD_BUTTON_L1:
			g_Renderer.ChangeLodBias(-0.1f);
			break;
		}


		/*
		if (camera.firstperson)
		{
		switch (wParam)
		{
		case 0x57:
		camera.keys.up = true;
		break;
		case 0x53:
		camera.keys.down = true;
		break;
		case 0x41:
		camera.keys.left = true;
		break;
		case 0x44:
		camera.keys.right = true;
		break;
		}
		}
		*/

		//keyPressed((uint32_t)wParam);
		break;
	case WM_KEYUP:
		/*
		if (camera.firstperson)
		{
		switch (wParam)
		{
		case 0x57:
		camera.keys.up = false;
		break;
		case 0x53:
		camera.keys.down = false;
		break;
		case 0x41:
		camera.keys.left = false;
		break;
		case 0x44:
		camera.keys.right = false;
		break;
		}
		}
		*/
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
		g_MousePos.x = (float)LOWORD(lParam);
		g_MousePos.y = (float)HIWORD(lParam);
		break;
	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_zoom += (float)wheelDelta * 0.005f * g_ZoomSpeed;
		//camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed));
		g_Renderer.UpdateUniformBuffers();
		g_Renderer.UpdateQuadUniformData();
		break;
	}
	case WM_MOUSEMOVE:
		if (wParam & MK_RBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_zoom += (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed;
			//camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)posy) * .005f * zoomSpeed));
			g_MousePos = glm::vec2((float)posx, (float)posy);

			g_Renderer.UpdateUniformBuffers();
			g_Renderer.UpdateQuadUniformData();
		}
		if (wParam & MK_LBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_Rotation.x += (g_MousePos.y - (float)posy) * 1.25f * g_RotationSpeed;
			g_Rotation.y -= (g_MousePos.x - (float)posx) * 1.25f * g_RotationSpeed;
			//camera.rotate(glm::vec3((mousePos.y - (float)posy), -(mousePos.x - (float)posx), 0.0f));
			g_MousePos = glm::vec2((float)posx, (float)posy);

			g_Renderer.UpdateUniformBuffers();
			g_Renderer.UpdateQuadUniformData();
		}
		if (wParam & MK_MBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_CameraPos.x -= (g_MousePos.x - (float)posx) * 0.01f;
			g_CameraPos.y -= (g_MousePos.y - (float)posy) * 0.01f;
			//camera.translate(glm::vec3(-(mousePos.x - (float)posx) * 0.01f, -(mousePos.y - (float)posy) * 0.01f, 0.0f));
			//viewChanged();
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