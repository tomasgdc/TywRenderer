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
#include <Renderer\ThirdParty\ImGui\imgui.h>


//math
#include <External\glm\glm\gtc\matrix_inverse.hpp>

//ImGui
#include <Renderer\ThirdParty\ImGui\imgui.h>


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



class Renderer final: public VKRenderer
{
private:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
	VkDescriptorSet descriptorSet;
	VkDescriptorSetLayout descriptorSetLayout;
	VkCommandBuffer  cmdGui;

	struct
	{
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
		// Text overlay submission and execution
		VkSemaphore textOverlayComplete;
		//Imgui sumbission and excetuion
		VkSemaphore guiComplete;
	} Semaphores;
public:
	RenderModelStatic			  staticModel;
	VkTools::VulkanTexture		  m_VkTexture;

	std::vector<VkBufferObject_s> listLocalBuffers;
	std::vector<uint32_t>		  meshSize;

	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 normal;
		glm::vec4 viewPos;
		glm::vec3 lightPos;
		float loadBias;
	}m_uboVS;

	struct
	{
		glm::vec3 lightPos;
		glm::vec3 Kd;
		glm::vec3 Ks;
		float	  ScreenGamma;
	}m_UniformShaderData;

	struct
	{
		VkTools::UniformData shaderData;
		VkTools::UniformData uniformDataVS;
	}m_descriptors;

	uint32_t numVerts;
	uint32_t numUvs;
	uint32_t numNormals;
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
	void StartFrame() override;
	void PrepareSemaphore() override;
	void LoadGUI() override;
	void LoadAssets() override;


	void SetupShaderData();
	void ImguiRender();
};


void Renderer::LoadAssets()
{
	VLoadTexture(GetAssetPath() + "Textures/cubemap_yokohama.ktx", VK_FORMAT_BC3_UNORM_BLOCK, false, true);
}

void Renderer::SetupShaderData()
{
	m_UniformShaderData.lightPos = glm::vec3(0.0f, 0.0f, 0.0f);
	m_UniformShaderData.Kd = glm::vec3(0.0f, 0.0f, 0.5f);
	m_UniformShaderData.Ks = glm::vec3(0.0f, 0.0f,  1.0f);
	m_UniformShaderData.ScreenGamma = 2.2f;

	m_uboVS.lightPos = m_UniformShaderData.lightPos;
}


void Renderer::PrepareSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;

	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.presentComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.renderComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.textOverlayComplete));
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.guiComplete));
}


Renderer::Renderer()
{

}

Renderer::~Renderer()
{

	//Delete memory
	for (int i = 0; i < listLocalBuffers.size(); i++)
	{
		VkBufferObject::DeleteBufferMemory(m_pWRenderer->m_SwapChain.device, listLocalBuffers[i], nullptr);
	}

	//Delete data from static model
	staticModel.Clear(m_pWRenderer->m_SwapChain.device);


	//Destroy Shader Module
	for (int i = 0; i < m_ShaderModules.size(); i++)
	{
		vkDestroyShaderModule(m_pWRenderer->m_SwapChain.device, m_ShaderModules[i], nullptr);
	}

	//destroy uniform data
	vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.buffer, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.memory, nullptr);

	vkDestroyBuffer(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.buffer, nullptr);
	vkFreeMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.memory, nullptr);

	//Release semaphores
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.presentComplete, nullptr);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.renderComplete, nullptr);
	vkDestroySemaphore(m_pWRenderer->m_SwapChain.device, Semaphores.textOverlayComplete, nullptr);

	//DestroyPipeline
	vkDestroyPipeline(m_pWRenderer->m_SwapChain.device, pipeline, nullptr);
	vkDestroyPipelineLayout(m_pWRenderer->m_SwapChain.device, pipelineLayout, nullptr);
	vkDestroyDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, descriptorSetLayout, nullptr);
}




void Renderer::BuildCommandBuffers()
{
	VkCommandBufferAllocateInfo cmdBufAllocateInfo = VkTools::Initializer::CommandBufferAllocateInfo(m_pWRenderer->m_CmdPool,VK_COMMAND_BUFFER_LEVEL_PRIMARY,1);
	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &cmdGui));


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
	renderPassBeginInfo.renderArea.extent.width = m_WindowWidth;
	renderPassBeginInfo.renderArea.extent.height = m_WindowHeight;
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
		viewport.height = (float)m_WindowHeight;
		viewport.width = (float)m_WindowWidth;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &viewport);

		// Update dynamic scissor state
		VkRect2D scissor = {};
		scissor.extent.width = m_WindowWidth;
		scissor.extent.height = m_WindowHeight;
		scissor.offset.x = 0;
		scissor.offset.y = 0;
		vkCmdSetScissor(m_pWRenderer->m_DrawCmdBuffers[i], 0, 1, &scissor);


		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline
		// So once we bind a pipeline all states that were set upon creation of that
		// pipeline will be set
		vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);


		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };

		// Bind descriptor sets describing shader binding points
		vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSet, 0, NULL);
		for (int j = 0; j < listLocalBuffers.size(); j++)
		{
			//Bind Buffer
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &listLocalBuffers[j].buffer, offsets);

			//Draw
			vkCmdDraw(m_pWRenderer->m_DrawCmdBuffers[i], meshSize[j], 1, 0, 0);
		}


		vkCmdEndRenderPass(m_pWRenderer->m_DrawCmdBuffers[i]);

		/*
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

		*/
		VK_CHECK_RESULT(vkEndCommandBuffer(m_pWRenderer->m_DrawCmdBuffers[i]));

	}
}

void Renderer::UpdateUniformBuffers()
{

	{
		m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 256.0f);
		m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 5.0f, g_zoom));

		m_uboVS.modelMatrix = m_uboVS.viewMatrix * glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, 5.0f));
		m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
		m_uboVS.normal = (glm::inverseTranspose(m_uboVS.modelMatrix));

		m_uboVS.viewPos = glm::vec4(0.0f, 0.0f, -15.0f, 0.0f);

		// Map uniform buffer and update it
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.memory, 0, sizeof(m_uboVS), 0, (void **)&pData));
		memcpy(pData, &m_uboVS, sizeof(m_uboVS));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.memory);
	}


	{
		// Map uniform buffer and update it
		uint8_t *pData;
		VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.memory, 0, sizeof(m_UniformShaderData), 0, (void **)&pData));
		memcpy(pData, &m_UniformShaderData, sizeof(m_UniformShaderData));
		vkUnmapMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.memory);
	}
}

void Renderer::PrepareUniformBuffers()
{
	//Set data first
	SetupShaderData();

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
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &m_descriptors.uniformDataVS.buffer));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(m_descriptors.uniformDataVS.memory)));
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.uniformDataVS.buffer, m_descriptors.uniformDataVS.memory, 0));

		m_descriptors.uniformDataVS.descriptor.buffer = m_descriptors.uniformDataVS.buffer;
		m_descriptors.uniformDataVS.descriptor.offset = 0;
		m_descriptors.uniformDataVS.descriptor.range = sizeof(m_uboVS);
	}


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
		bufferInfo.size = sizeof(m_UniformShaderData);
		bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

		// Create a new buffer
		VK_CHECK_RESULT(vkCreateBuffer(m_pWRenderer->m_SwapChain.device, &bufferInfo, nullptr, &m_descriptors.shaderData.buffer));
		vkGetBufferMemoryRequirements(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.buffer, &memReqs);
		allocInfo.allocationSize = memReqs.size;
		allocInfo.memoryTypeIndex = VkTools::GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_pWRenderer->m_DeviceMemoryProperties);
		VK_CHECK_RESULT(vkAllocateMemory(m_pWRenderer->m_SwapChain.device, &allocInfo, nullptr, &(m_descriptors.shaderData.memory)));
		VK_CHECK_RESULT(vkBindBufferMemory(m_pWRenderer->m_SwapChain.device, m_descriptors.shaderData.buffer, m_descriptors.shaderData.memory, 0));

		// Store information in the uniform's descriptor
		m_descriptors.shaderData.descriptor.buffer = m_descriptors.shaderData.buffer;
		m_descriptors.shaderData.descriptor.offset = 0;
		m_descriptors.shaderData.descriptor.range = sizeof(m_UniformShaderData);
	}

	UpdateUniformBuffers();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	staticModel.InitFromFile("Geometry/teapot/teapoTriangulated.obj", GetAssetPath());


	std::vector<VkBufferObject_s> listStagingBuffers;
	listLocalBuffers.resize(staticModel.surfaces.size());
	listStagingBuffers.resize(staticModel.surfaces.size());

	for (int i = 0; i < staticModel.surfaces.size(); i++) 
	{
		//Get triangles
		srfTriangles_t* tr = staticModel.surfaces[i].geometry;


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
	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {};
	pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineCreateInfo.layout = pipelineLayout;
	pipelineCreateInfo.renderPass = m_pWRenderer->m_RenderPass;

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
			VK_SAMPLE_COUNT_4_BIT,
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
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/PhysicalBasedShading/mesh.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/PhysicalBasedShading/mesh.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


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
}



void Renderer::VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling, bool bUseCubeMap)
{
	if (!bUseCubeMap)
		m_pTextureLoader->LoadTexture(fileName, format, &m_VkTexture);
	else
		m_pTextureLoader->LoadCubemap(fileName, format, &m_VkTexture);
}


void Renderer::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0),
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_FRAGMENT_BIT,1),
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2),
	};

	VkDescriptorSetLayoutCreateInfo descriptorLayout = VkTools::Initializer::DescriptorSetLayoutCreateInfo(setLayoutBindings.data(), setLayoutBindings.size());
	VK_CHECK_RESULT(vkCreateDescriptorSetLayout(m_pWRenderer->m_SwapChain.device, &descriptorLayout, nullptr, &descriptorSetLayout));

	VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = VkTools::Initializer::PipelineLayoutCreateInfo(&descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkCreatePipelineLayout(m_pWRenderer->m_SwapChain.device, &pPipelineLayoutCreateInfo, nullptr, &pipelineLayout));
}

void Renderer::SetupDescriptorPool()
{
	// Example uses one ubo and one image sampler
	std::vector<VkDescriptorPoolSize> poolSizes =
	{
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2),
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(poolSizes.size(),poolSizes.data(), 1);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_pWRenderer->m_SwapChain.device, &descriptorPoolInfo, nullptr, &m_pWRenderer->m_DescriptorPool));
}


void Renderer::SetupDescriptorSet()
{
	VkDescriptorSetAllocateInfo allocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(m_pWRenderer->m_DescriptorPool, &descriptorSetLayout, 1);
	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &descriptorSet));
	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		//uniform descriptor
		VkTools::Initializer::WriteDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0, &m_descriptors.uniformDataVS.descriptor),
		VkTools::Initializer::WriteDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	1, &m_descriptors.shaderData.descriptor),
		VkTools::Initializer::WriteDescriptorSet(descriptorSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2, &m_VkTexture.descriptor)
	};
	//Update descriptor set
	vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
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
		//ImGUI Render
		ImGui_ImplGlfwVulkan_Render(cmdGui, m_pWRenderer->m_currentBuffer);


		//Submit model
		VkPipelineStageFlags submitPipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkPipelineStageFlags stageFlags = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();

		//Render model
		{
			submitInfo.commandBufferCount = 1;
			submitInfo.signalSemaphoreCount = 1;
			submitInfo.waitSemaphoreCount = 1;

			submitInfo.pWaitDstStageMask = &submitPipelineStages;
			submitInfo.pWaitSemaphores = &Semaphores.presentComplete;

			submitInfo.pSignalSemaphores = &Semaphores.renderComplete;
			submitInfo.pCommandBuffers = &m_pWRenderer->m_DrawCmdBuffers[m_pWRenderer->m_currentBuffer];
			VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
		}
		
		//Now Render GUI
		{
			submitInfo.pWaitDstStageMask = &stageFlags;

			submitInfo.pCommandBuffers = &cmdGui;
			submitInfo.pWaitSemaphores = &Semaphores.renderComplete;
			submitInfo.pSignalSemaphores = &Semaphores.guiComplete;
			VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
		}
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
		VK_CHECK_RESULT(m_pWRenderer->m_SwapChain.QueuePresent(m_pWRenderer->m_Queue, m_pWRenderer->m_currentBuffer, Semaphores.guiComplete));
	}
}


void Renderer::LoadGUI()
{
	//Load Imgui
	ImGui_ImplGlfwVulkan_Init(m_pWRenderer, &m_WindowWidth, &m_WindowHeight, false);

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



void Renderer::ImguiRender()
{
		ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
		ImGui::Text("%.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::DragFloat3("Kd", (float*)&m_UniformShaderData.Kd, 0.0f, 0.0f, 50.0f, "%.3f");
		ImGui::DragFloat3("Ks", (float*)&m_UniformShaderData.Ks, 0.0f, 0.0f, 50.0f, "%.3f");
		ImGui::DragFloat3("Light Position", (float*)&m_UniformShaderData.lightPos, 0.0f, 0.0f, 50.0f, "%.3f");
		ImGui::DragFloat("Screen Gamma ", &m_UniformShaderData.ScreenGamma, 0.0f, 0.0f, 2.4f, "%.3f");

		m_uboVS.lightPos = m_UniformShaderData.lightPos;
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

	double tDiff = 0;
	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();
		ImGui_ImplGlfwVulkan_NewFrame(g_Renderer.frameTimer);
		g_Renderer.ImguiRender();

		g_Renderer.StartFrame();
		//Do something
		g_Renderer.EndFrame(nullptr);


		auto tEnd = std::chrono::high_resolution_clock::now();
		g_Renderer.frameCounter++;
		tDiff = std::chrono::duration<double, std::milli>(tEnd - tStart).count();
		g_Renderer.frameTimer = (float)tDiff / 1000.0f;
		g_Renderer.fpsTimer += (float)tDiff;
		if (g_Renderer.fpsTimer > 1000.0f)
		{
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
		case VK_F1:
			break;
		case VK_ESCAPE:
		{
			PostQuitMessage(0);
			break;
		}
		break;
	case WM_KEYUP:
		break;
	case WM_RBUTTONDOWN:
	case WM_LBUTTONDOWN:
		g_MousePos.x = (float)LOWORD(lParam);
		g_MousePos.y = (float)HIWORD(lParam);
		ImGui_ImplGlfwVulkan_MousePressed(true);
		break;
	case WM_LBUTTONUP:
		ImGui_ImplGlfwVulkan_MousePressed(false);
		break;
	case WM_MBUTTONDOWN:
		break;
	case WM_MOUSEWHEEL:
	{
		short wheelDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		g_zoom += (float)wheelDelta * 0.005f * g_ZoomSpeed;
		//camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed));
		g_Renderer.UpdateUniformBuffers();
		break;
	}
	case WM_MOUSEMOVE:
		ImGui_ImplGlfwVulkan_SetMousePos(LOWORD(lParam), HIWORD(lParam));

		if (wParam & MK_RBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_zoom += (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed;
			//camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)posy) * .005f * zoomSpeed));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			
			
			g_Renderer.UpdateUniformBuffers();
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