/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/


#include <RendererPch\stdafx.h>

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
#include <Renderer\AnimationManager\MD5Anim\MD5Anim.h>


//Vulkan Includes
#include <Renderer\Vulkan\VkBufferObject.h>
#include <Renderer\Vulkan\VulkanSwapChain.h>


//Font Rendering
#include <Renderer\ThirdParty\FreeType\VkFont.h>






//Global variables
//====================================================================================

uint32_t	g_iDesktopWidth = 0;
uint32_t	g_iDesktopHeight = 0;
bool		g_bPrepared = false;

glm::vec3	g_Rotation = glm::vec3();
glm::vec3	g_CameraPos = glm::vec3();
glm::vec2	g_MousePos;

//timer



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
	VkTools::VulkanTexture m_VkTexture;
	VkFont*	m_VkFont;

	struct {
		glm::mat4 bones[110];
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::mat4 normal;
		glm::vec4 viewPos;
		float lodBias = 0.0f;
	}m_uboVS;

	uint32_t numVerts = 0;
	uint32_t numUvs = 0;
	uint32_t numNormals = 0;

	//Animation
	float		m_fDeltaTime;
	frameBlend_t frame;

	std::vector<VkBufferObject_s> listLocalBuffers;
	std::vector<VkBufferObject_s> listLocalIndexBuffers;
	std::vector<VkBufferObject_s> listStagingBuffersVerts;


	std::vector<VkDescriptorSet>  listDescriptros;
	std::vector<uint32_t>		   meshSize;

	//Model loading
	RenderModelMD5 md5Model;
	MD5Anim		   md5Anim;
public:
	Renderer();
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

	bool m_bViewChanged = false;

	void BeginTextUpdate();
};


Renderer::Renderer()
{
	frame = { 0, 0, 0.0f };
}

Renderer::~Renderer()
{
	SAFE_DELETE(m_VkFont);
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

	m_bViewChanged = true;
	UpdateUniformBuffers();
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

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pWRenderer->m_DrawCmdBuffers[m_pWRenderer->m_currentBuffer];

		// Wait for swap chain presentation to finish
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &Semaphores.presentComplete;

		//Signal ready for model render to complete
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &Semaphores.renderComplete;
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));


		//Wait for color output before rendering text
		submitInfo.pWaitDstStageMask = &stageFlags;

		// Wait model rendering to finnish
		submitInfo.pWaitSemaphores = &Semaphores.renderComplete;
		//Signal ready for text to completeS
		submitInfo.pSignalSemaphores = &Semaphores.textOverlayComplete;

		submitInfo.pCommandBuffers = &m_VkFont->cmdBuffers[m_pWRenderer->m_currentBuffer];
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));


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


void Renderer::BuildCommandBuffers()
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


		// Bind the rendering pipeline
		// The pipeline (state object) contains all states of the rendering pipeline
		// So once we bind a pipeline all states that were set upon creation of that
		// pipeline will be set
		vkCmdBindPipeline(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);


		// Bind triangle vertex buffer (contains position and colors)
		VkDeviceSize offsets[1] = { 0 };

		for (int j = 0; j < listLocalBuffers.size(); j++)
		{
			// Bind descriptor sets describing shader binding points
			vkCmdBindDescriptorSets(m_pWRenderer->m_DrawCmdBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &listDescriptros[j], 0, NULL);

			//Bind Buffer
			vkCmdBindVertexBuffers(m_pWRenderer->m_DrawCmdBuffers[i], VERTEX_BUFFER_BIND_ID, 1, &listLocalBuffers[j].buffer, offsets);

			//Bind mesh index buffers
			vkCmdBindIndexBuffer(m_pWRenderer->m_DrawCmdBuffers[i], listLocalIndexBuffers[j].buffer, 0, VK_INDEX_TYPE_UINT32);

			//Draw
			vkCmdDrawIndexed(m_pWRenderer->m_DrawCmdBuffers[i], meshSize[j], 1, 0, 0, 0);
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


void Renderer::BeginTextUpdate()
{
	m_VkFont->BeginTextUpdate();

	std::stringstream ss;
	ss << std::fixed << std::setprecision(2) << "ms-" << (frameTimer * 1000.0f) << "-fps-" << lastFPS;
	m_VkFont->AddText(-25, -30, 0.1, 0.1, ss.str());

	m_VkFont->EndTextUpdate();
}


void Renderer::UpdateUniformBuffers()
{
	if (m_bViewChanged)
	{
		// Update matrices
		m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 256.0f);

		m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 5.0f, g_zoom));
		//m_uboVS.viewMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		//m_uboVS.viewMatrix *= glm::rotate(m_uboVS.modelMatrix, glm::radians(-91.25f), glm::vec3(0.0f, 1.0f, 0.0f));
		//m_uboVS.viewMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		m_uboVS.modelMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.7f));
		m_uboVS.modelMatrix *= glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, -2.0f, -5.0f));
	    m_uboVS.modelMatrix *= glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
		m_uboVS.modelMatrix *= glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
		m_uboVS.modelMatrix *= glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

		m_uboVS.normal = glm::inverseTranspose(m_uboVS.modelMatrix);
		m_uboVS.viewPos =  glm::vec4(0.0f, 0.0f, -15.0f, 0.0f);

		//reset to normal
		m_bViewChanged = false;
	}
	
	md5Anim.ConvertDeltaTimeToFrame(frameTimer, frame);
	md5Anim.GetInterpolatedFrame(frame);
	const std::vector<glm::mat4x4>& matrix = md5Anim.GetSkeletonMatrix();
	for (int i = 0; i < matrix.size(); i++)
	{
		m_uboVS.bones[i] = matrix[i] * md5Model.inverseBindPose[i];
	}



	// Map uniform buffer and update it
	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(m_uboVS), 0, (void **)&pData));
	memcpy(pData, &m_uboVS, sizeof(m_uboVS));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory);
}

void Renderer::PrepareUniformBuffers()
{
	// Prepare and initialize a uniform buffer block containing shader uniforms
	// In Vulkan there are no more single uniforms like in GL
	// All shader uniforms are passed as uniform buffer blocks 
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

	UpdateUniformBuffers();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	//Create font pipeline
	m_VkFont->CreateFontVk((GetAssetPath() + "Textures/freetype/AmazDooMLeft.ttf"), 64, 96);

	m_VkFont->SetupDescriptorPool();
	m_VkFont->SetupDescriptorSetLayout();
	m_VkFont->PrepareUniformBuffers();
	m_VkFont->InitializeChars("qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM-:.@1234567890", *m_pTextureLoader);
	m_VkFont->PrepareResources(g_iDesktopWidth, g_iDesktopHeight);
	BeginTextUpdate();

	std::vector<VkBufferObject_s> listStagingBuffersIndex;

	//Verts
	listStagingBuffersVerts.resize(md5Model.meshes.size());
	listStagingBuffersVerts.resize(md5Model.meshes.size());
	listStagingBuffersVerts.resize(md5Model.meshes.size());


	//Indexes
	listStagingBuffersIndex.resize(md5Model.meshes.size());
	listStagingBuffersIndex.resize(md5Model.meshes.size());
	listStagingBuffersIndex.resize(md5Model.meshes.size());


	listLocalBuffers.resize(md5Model.meshes.size());
	listLocalIndexBuffers.resize(md5Model.meshes.size());
	listDescriptros.resize(md5Model.meshes.size());

	listDescriptros.resize(md5Model.meshes.size());
	m_pWRenderer->m_DescriptorPool = VK_NULL_HANDLE;
	SetupDescriptorPool();
	VkDescriptorSetAllocateInfo allocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(m_pWRenderer->m_DescriptorPool, &descriptorSetLayout, 1);



	for (int i = 0; i < md5Model.meshes.size(); i++)
	{
		//Get triangles
		//deformInfo_t* tr = md5Model.meshes[i].deformInfo;

		std::vector<MD5Mesh::meshStructure>&	 deformInfosVec = md5Model.meshes[i].deformInfosVec;
		std::vector<uint32_t>& indexes = md5Model.meshes[i].tri;
		uint32_t numVerts = md5Model.meshes[i].verts.size();


		VkTools::VulkanTexture* vkDiffuseTexture = md5Model.meshes[i].shader->getTexture();
		/*
			=================================================================================================================
			START SETUP DESCRIPTOR SET
			=================================================================================================================
		*/
		VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &listDescriptros[i]));

		// Image descriptor for the color map texture
		VkDescriptorImageInfo texDescriptorDiffuse = VkTools::Initializer::DescriptorImageInfo(vkDiffuseTexture->sampler, vkDiffuseTexture->view, VK_IMAGE_LAYOUT_GENERAL);
		//VkDescriptorImageInfo texDescriptorNormal = VkTools::Initializer::DescriptorImageInfo(vkBumpTexture->sampler, vkBumpTexture->view, VK_IMAGE_LAYOUT_GENERAL);

		std::vector<VkWriteDescriptorSet> writeDescriptorSets =
		{
			// Binding 0 : Vertex shader uniform buffer
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0,&uniformDataVS.descriptor),

			// Binding 1 : Fragment shader texture sampler
			VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&texDescriptorDiffuse),

			// Binding 2 : Fragment shader texture sampler
			//VkTools::Initializer::WriteDescriptorSet(listDescriptros[i],VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2,&texDescriptorNormal)
		};
		vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);

		/*
		=================================================================================================================
		END SETUP DESCRIPTOR SET
		=================================================================================================================
		*/

		//Create stagign buffer
		//Verts
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			static_cast<uint32_t>(sizeof(MD5Mesh::meshStructure) * numVerts),
			listStagingBuffersVerts[i],
			deformInfosVec.data());

		//Index
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			static_cast<uint32_t>(sizeof(uint32_t) * indexes.size()),
			listStagingBuffersIndex[i],
			indexes.data());


		//Create Local Copy
		//Verts
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			static_cast<uint32_t>(sizeof(MD5Mesh::meshStructure) * numVerts),
			listLocalBuffers[i]);

		//Index
		VkBufferObject::CreateBuffer(
			m_pWRenderer->m_SwapChain,
			m_pWRenderer->m_DeviceMemoryProperties,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			static_cast<uint32_t>(sizeof(uint32_t) * indexes.size()),
			listLocalIndexBuffers[i]);


		//Create new command buffer
		VkCommandBuffer copyCmd = GetCommandBuffer(true);


		//Submit info to the queue
		VkBufferObject::SubmitBufferObjects(
			copyCmd,
			m_pWRenderer->m_Queue,
			*m_pWRenderer,
			static_cast<uint32_t>(sizeof(MD5Mesh::meshStructure) * numVerts),
				listStagingBuffersVerts[i],
				listLocalBuffers[i], drawVertFlags::None);



		// Binding description
		listLocalBuffers[i].bindingDescriptions.resize(1);
		listLocalBuffers[i].bindingDescriptions[0].binding = 0;
		listLocalBuffers[i].bindingDescriptions[0].stride = sizeof(MD5Mesh::meshStructure);
		listLocalBuffers[i].bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Attribute descriptions
		// Describes memory layout and shader attribute locations
		listLocalBuffers[i].attributeDescriptions.resize(6);

		// Location 0 : Position
		listLocalBuffers[i].attributeDescriptions[0].binding = 0;
		listLocalBuffers[i].attributeDescriptions[0].location = 0;
		listLocalBuffers[i].attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		listLocalBuffers[i].attributeDescriptions[0].offset = offsetof(MD5Mesh::meshStructure, vertex);

		// Location 1 : Uv
		listLocalBuffers[i].attributeDescriptions[1].binding = 0;
		listLocalBuffers[i].attributeDescriptions[1].location = 1;
		listLocalBuffers[i].attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
		listLocalBuffers[i].attributeDescriptions[1].offset = offsetof(MD5Mesh::meshStructure, tex);

		// Location 2 : BoneWeight 1
		listLocalBuffers[i].attributeDescriptions[2].binding = 0;
		listLocalBuffers[i].attributeDescriptions[2].location = 2;
		listLocalBuffers[i].attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		listLocalBuffers[i].attributeDescriptions[2].offset = offsetof(MD5Mesh::meshStructure, boneWeight1);

		
		// Location 3 : BoneWeight 2
		listLocalBuffers[i].attributeDescriptions[3].binding = 0;
		listLocalBuffers[i].attributeDescriptions[3].location = 3;
		listLocalBuffers[i].attributeDescriptions[3].format = VK_FORMAT_R32G32B32A32_SFLOAT;
		listLocalBuffers[i].attributeDescriptions[3].offset = offsetof(MD5Mesh::meshStructure, boneWeight2);
		


		// Location 4 : jointId 1
		listLocalBuffers[i].attributeDescriptions[4].binding = 0;
		listLocalBuffers[i].attributeDescriptions[4].location = 4;
		listLocalBuffers[i].attributeDescriptions[4].format = VK_FORMAT_R32G32B32A32_SINT;
		listLocalBuffers[i].attributeDescriptions[4].offset = offsetof(MD5Mesh::meshStructure, boneId1);

		
		// Location 5 : jointId 2
		listLocalBuffers[i].attributeDescriptions[5].binding = 0;
		listLocalBuffers[i].attributeDescriptions[5].location = 5;
		listLocalBuffers[i].attributeDescriptions[5].format = VK_FORMAT_R32G32B32A32_SINT;
		listLocalBuffers[i].attributeDescriptions[5].offset = offsetof(MD5Mesh::meshStructure, boneId2);
		




		// Assign to vertex input state
		listLocalBuffers[i].inputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		listLocalBuffers[i].inputState.pNext = NULL;
		listLocalBuffers[i].inputState.flags = VK_FLAGS_NONE;
		listLocalBuffers[i].inputState.vertexBindingDescriptionCount = static_cast<uint32_t>(listLocalBuffers[i].bindingDescriptions.size());
		listLocalBuffers[i].inputState.pVertexBindingDescriptions = listLocalBuffers[i].bindingDescriptions.data();
		listLocalBuffers[i].inputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(listLocalBuffers[i].attributeDescriptions.size());
		listLocalBuffers[i].inputState.pVertexAttributeDescriptions = listLocalBuffers[i].attributeDescriptions.data();



		copyCmd = GetCommandBuffer(true);
		VkBufferObject::SubmitBufferObjects(
			copyCmd,
			m_pWRenderer->m_Queue,
			*m_pWRenderer,
			static_cast<uint32_t>(sizeof(uint32_t) * indexes.size()),
			listStagingBuffersIndex[i],
			listLocalIndexBuffers[i], drawVertFlags::None);


		meshSize.push_back(indexes.size());
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
	shaderStages[0] = LoadShader(GetAssetPath() + "Shaders/SkeletalAnimation/cpuAnimation.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	shaderStages[1] = LoadShader(GetAssetPath() + "Shaders/SkeletalAnimation/cpuAnimation.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);


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



void Renderer::VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling)
{
	m_pTextureLoader->LoadTexture(fileName, format, &m_VkTexture);
}


void Renderer::SetupDescriptorSetLayout()
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,VK_SHADER_STAGE_VERTEX_BIT,0),

		// Binding 1 : Fragment shader image sampler
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,1),

		// Binding 2 : Fragment shader image sampler
		VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,VK_SHADER_STAGE_FRAGMENT_BIT,2)
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
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 7),
		VkTools::Initializer::DescriptorPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 7*2)
	};

	VkDescriptorPoolCreateInfo descriptorPoolInfo = VkTools::Initializer::DescriptorPoolCreateInfo(poolSizes.size(),poolSizes.data(),7);
	VK_CHECK_RESULT(vkCreateDescriptorPool(m_pWRenderer->m_SwapChain.device, &descriptorPoolInfo, nullptr, &m_pWRenderer->m_DescriptorPool));
}


void Renderer::SetupDescriptorSet()
{
	/*
	VkDescriptorSetAllocateInfo allocInfo = VkTools::Initializer::DescriptorSetAllocateInfo(m_pWRenderer->m_DescriptorPool,&descriptorSetLayout,1);

	VK_CHECK_RESULT(vkAllocateDescriptorSets(m_pWRenderer->m_SwapChain.device, &allocInfo, &descriptorSet));

	// Image descriptor for the color map texture
	VkDescriptorImageInfo texDescriptor = VkTools::Initializer::DescriptorImageInfo(m_VkTexture.sampler, m_VkTexture.view,VK_IMAGE_LAYOUT_GENERAL);

	std::vector<VkWriteDescriptorSet> writeDescriptorSets =
	{
		// Binding 0 : Vertex shader uniform buffer
		VkTools::Initializer::WriteDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,	0,&uniformDataVS.descriptor),
	
		// Binding 1 : Fragment shader texture sampler
		VkTools::Initializer::WriteDescriptorSet(descriptorSet,VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1,&texDescriptor)
	};

	vkUpdateDescriptorSets(m_pWRenderer->m_SwapChain.device, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, NULL);
	*/
}




void Renderer::LoadAssets()
{
	m_VkFont = TYW_NEW VkFont(m_pWRenderer->m_SwapChain.physicalDevice, m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_Queue, m_pWRenderer->m_FrameBuffers, m_pWRenderer->m_SwapChain.colorFormat, m_pWRenderer->m_SwapChain.depthFormat, &m_WindowWidth, &m_WindowHeight);
	md5Model.InitFromFile("Geometry/hellknight/hellknight.md5mesh", GetAssetPath());
	md5Anim.LoadAnim("Animation/hellknight/idle2.md5anim", GetAssetPath());
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
		g_Renderer.StartFrame();
		g_Renderer.UpdateUniformBuffers();
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
		
		g_Renderer.m_bViewChanged = true;
		g_Renderer.UpdateUniformBuffers();
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

			g_Renderer.m_bViewChanged = true;
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

			g_Renderer.m_bViewChanged = true;
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
		if (g_bPrepared) {
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