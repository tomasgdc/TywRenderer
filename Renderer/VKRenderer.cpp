/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#include <RendererPch\stdafx.h>

//Renderer Includes
#include "VKRenderer.h"

//Vulkan Includes
#include "Vulkan\VulkanTextureLoader.h"

//MeshLoader Includes
#include "MeshLoader\ImageManager.h"


//Font Incldues
#include "Geometry\VertData.h"
#include "Vulkan\VkBufferObject.h"
#include "ThirdParty\FreeType\VkFont.h"


#define VERTEX_BUFFER_BIND_ID 0
// Set to "true" to enable Vulkan's validation layers
// See vulkandebug.cpp for details
#define ENABLE_VALIDATION false
// Set to "true" to use staging buffers for uploading
// vertex and index data to device local memory
// See "prepareVertices" for details on what's staging
// and on why to use it
#define USE_STAGING true


VKRenderer::VKRenderer(): m_pWRenderer(nullptr), m_bIsOpenglRunning(false)
{
#ifdef _DEBUG
	m_bLogRenderer = true;
#endif
}

VKRenderer::~VKRenderer()
{
	SAFE_DELETE(m_pTextureLoader);
	SAFE_DELETE(m_pImageManager);
	SAFE_DELETE(m_pWRenderer);
}

void VKRenderer::VShutdown()
{
	ImGui_ImplGlfwVulkan_Shutdown();
	m_pWRenderer->DestroyRendererScreen();
}

bool VKRenderer::VPreRender()
{
	return true;
}


bool VKRenderer::VPostRender()
{
	return true;
}

bool VKRenderer::VInitRenderer(uint32_t height, uint32_t width, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM))
{

	m_WindowHeight = height;
	m_WindowWidth = width;

	//Initialize Logging
	VSetLogPath();

	m_pWRenderer = TYW_NEW VulkanRendererInitializer;

	//Create Screen
	if (!m_pWRenderer->CreateRendererScreen(height, width, isFullscreen, MainWindowProc))
	{
		Logv("Error: Could not create window screen for renderer at %s line %d \n", __FILE__, __LINE__);
		return false;
	}


	//Initialize Texture Loader
	m_pTextureLoader = TYW_NEW VkTools::VulkanTextureLoader(m_pWRenderer->m_SwapChain.physicalDevice, m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_Queue, m_pWRenderer->m_CmdPool);
	globalImage = TYW_NEW ImageManager(m_pWRenderer->m_SwapChain.physicalDevice, m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_Queue, m_pWRenderer->m_CmdPool);

	//Load all needed assets. Overrided
	//Models, textures and so on
	LoadAssets();

	//Prepare semaphores for synchronization
	PrepareSemaphore();

	//Prepare all descriptors firstly. Describe shader uniforms, what texture is used and so on
	//Can go after preparing vertices as it not influences it. Because you descripe your GLSL shader.
	//Similar to glGetUniformLocation .. but gives more options to describe.
	//For texture you use glUniform1i(texLoc, 0); and glActiveTexture(GL_TEXTURE0); to say which uniform
	//In Vulkan you use layout (binding = 1) uniform sampler2D samplerColor; and attach binding numb to DescriptorLayout where you describe what kind of data
	//this binding point represents. Is it uniform buffer or image  and so on...
	PrepareUniformBuffers();
	SetupDescriptorSetLayout();
	SetupDescriptorPool();
	SetupDescriptorSet();

	//Prepare vertices -> Send data to GPU
	//Similar to 	glGenBuffers(1, &m_vbo);  glBindBuffer(GL_ARRAY_BUFFER, m_vbo); glBufferData(GL_ARRAY_BUFFER, allocSize, nullptr, GL_STATIC_DRAW);
	//In Vulkan you create staging buffer(depends what you choose. To send data to gpu we use so
	//called staging buffer), describe needed buffer size of object you pass(5k vertices * vec3) and create handle for VkBuffer
	//Then you copy to your local buffer from stagging and send it to vk queue.
	//Do not forget to describe your data layout 
	//localBuffer.attributeDescriptions[0].binding = 0;
	//localBuffer.attributeDescriptions[0].location = 0;
	//localBuffer.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
	//localBuffer.attributeDescriptions[0].offset = offsetof(drawVert, vertex);
	//similar to
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(drawVert), (void*)offsetof(drawVert, vertex));
	PrepareVertices(USE_STAGING);

	//Create Pipeline state VI-IA-VS-VP-RS-FS-CB
	//Encapsulates fixed states
	//We get rid of glEnable(GL_BLEND); 	glEnable(GL_CULL_FACE); glEnable(GL_DEPTH_TEST); and so on
	//A pipeline is then stored and hashed on the GPU making
	//pipeline changes much faster than having to set dozens of 
	//states
	//Each GLSL shader pass will have pipeline
	PreparePipeline();
	
	//Create RenderPass
	//Build Commands for Rendering
	//Bind Correct VkBuffer for drawing and VkPipeline -> Call draw command
	//End RenderPass
	BuildCommandBuffers();


	//Load GUI
	LoadGUI();

	m_bIsOpenglRunning = true;
	return true;
}

void VKRenderer::LoadGUI()
{
	//overriden
}

void VKRenderer::LoadAssets()
{
	//overrideen
}

HRESULT VKRenderer::VOnRestore()
{
	return IDH_OK;
}





std::shared_ptr<IRenderState> VKRenderer::VPrepareAlphaPass()
{
	return nullptr;
}


std::shared_ptr<IRenderState> VKRenderer::VPrepareSkyBoxPass()
{
	return nullptr;
}


void VKRenderer::StartFrame()
{
	//overriden
}


void VKRenderer::EndFrame(uint64_t* gpuMicroSec)
{
	VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));
}


void VKRenderer::SwapCommandBuffers_FinnishRendering(uint64_t* gpuMicroSec)
{
	if (gpuMicroSec != nullptr) {
		*gpuMicroSec = 0;
	}

	if (!m_bIsOpenglRunning) {
		return;
	}

	//if there is rendering command -> swapBuffer
	m_pWRenderer->RendererSwapBuffers();

	//TODO: -> m_glConfigs.timerQueryAvailable
	if (true) 
	{
		uint64_t drawingTimeNanoseconds = 0;
		if (m_iTimerQueryId != 0) 
		{
			//glGetQueryObjectui64v(m_iTimerQueryId, GL_QUERY_RESULT, &drawingTimeNanoseconds);
		}
		if (gpuMicroSec != nullptr) 
		{
			*gpuMicroSec = drawingTimeNanoseconds / 1000;
		}
	}
}



//TODO: Create File management system
void VKRenderer::VSetLogPath()
{
	m_LogFile = fopen("../LogInfo/GLRendererLog.txt", "w");
	if (!m_LogFile)
	{
		perror("Could not open GLRenderLog.txt");
	}

	m_LogFileStr = fopen("../LogInfo/GLLogStreaming.txt", "w");
	if (!m_LogFileStr)
	{
		perror("Could not open GLLogStreaming.txt");
	}

	m_LogFileSh = fopen("../LogInfo/GLShaderLog.txt", "w");
	if (!m_LogFileStr)
	{
		perror("Could not open GLShaderLog.txt");
	}
}

void  VKRenderer::Logv(const char* format, ...)
{
	va_list argptr;
	if (m_LogFile)
	{
		va_start(argptr, format);
		vfprintf(m_LogFile, format, argptr);
		va_end(argptr);
		fflush(m_LogFile);
	}
}

void   VKRenderer::LogStrv(char* format, ...)
{
	va_list argptr;
	if (m_LogFileStr)
	{
		fprintf(m_LogFileStr, "  ");
		va_start(argptr, format);
		vfprintf(m_LogFileStr, format, argptr);
		va_end(argptr);
		fflush(m_LogFileStr);
	}
}

void   VKRenderer::LogShv(char* format, ...)
{
	va_list argptr;
	if (m_LogFileSh)
	{
		va_start(argptr, format);
		vfprintf(m_LogFileSh, format, argptr);
		va_end(argptr);
		fflush(m_LogFileSh);
	}
}


void   VKRenderer::Log(char* str, ...)
{
	va_list argptr;
	va_start(argptr, str);
	vfprintf(stdout, str, argptr);
	va_end(argptr);
}


// Create synchronzation semaphores
void VKRenderer::PrepareSemaphore()
{
	//overriden
}

void VKRenderer::PreparePipeline()
{
	//overriden
}



VkPipelineShaderStageCreateInfo VKRenderer::LoadShader(std::string fileName, VkShaderStageFlagBits stage)
{
	VkPipelineShaderStageCreateInfo shaderStage = {};
	shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStage.stage = stage;
#if defined(__ANDROID__)
	shaderStage.module = vkTools::loadShader(androidApp->activity->assetManager, fileName.c_str(), device, stage);
#else
	shaderStage.module = VkTools::LoadShader(fileName.c_str(), m_pWRenderer->m_SwapChain.device, stage);
#endif
	shaderStage.pName = "main"; // todo : make param
	assert(shaderStage.module != NULL);
	m_ShaderModules.push_back(shaderStage.module);
	return shaderStage;
}


void VKRenderer::SetupDescriptorSetLayout()
{
	//overriden
}


void VKRenderer::SetupDescriptorPool()
{
	//overriden
}


// Build separate command buffers for every framebuffer image
// Unlike in OpenGL all rendering commands are recorded once
// into command buffers that are then resubmitted to the queue
void VKRenderer::BuildCommandBuffers()
{
	//overriden
}


	// Setups vertex and index buffers for an indexed triangle,
	// uploads them to the VRAM and sets binding points and attribute
	// descriptions to match locations inside the shaders
void VKRenderer::PrepareVertices(bool useStagingBuffers)
{
	//overriden
}



void VKRenderer::PrepareUniformBuffers()
{
	//overriden
}

void VKRenderer::UpdateUniformBuffers()
{
	//overriden
}



// We are going to use several temporary command buffers for setup stuff
// like submitting barriers for layout translations of buffer copies
// This function will allocate a single (primary) command buffer from the 
// examples' command buffer pool and if begin is set to true it will
// also start the buffer so we can directly put commands into it
VkCommandBuffer VKRenderer::GetCommandBuffer(bool begin)
{
	VkCommandBuffer cmdBuffer;

	VkCommandBufferAllocateInfo cmdBufAllocateInfo = {};
	cmdBufAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdBufAllocateInfo.commandPool = m_pWRenderer->m_CmdPool;
	cmdBufAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cmdBufAllocateInfo.commandBufferCount = 1;

	VK_CHECK_RESULT(vkAllocateCommandBuffers(m_pWRenderer->m_SwapChain.device, &cmdBufAllocateInfo, &cmdBuffer));

	// If requested, also start the new command buffer
	if (begin)
	{
		VkCommandBufferBeginInfo cmdBufInfo = VkTools::Initializer::CommandBufferBeginInfo();
		VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
	}

	return cmdBuffer;
}

// This will end the command buffer and submit it to the examples' queue
// Then waits for the queue to become idle so our submission is finished
// and then deletes the command buffer
// For use with setup command buffers created by getCommandBuffer
void VKRenderer::FlushCommandBuffer(VkCommandBuffer commandBuffer)
{
	assert(commandBuffer != VK_NULL_HANDLE);

	VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer));

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));
	VK_CHECK_RESULT(vkQueueWaitIdle(m_pWRenderer->m_Queue));

	vkFreeCommandBuffers(m_pWRenderer->m_SwapChain.device, m_pWRenderer->m_CmdPool, 1, &commandBuffer);
}

void VKRenderer::SetupDescriptorSet()
{
	//overriden
}


void VKRenderer::VWindowResize(uint32_t iHeight, uint32_t iWidth)
{
	m_WindowHeight = iHeight;
	m_WindowWidth = iWidth;

	m_pWRenderer->WindowResize(iWidth, iHeight);

	BuildCommandBuffers();
	vkQueueWaitIdle(m_pWRenderer->m_Queue);
	vkDeviceWaitIdle(m_pWRenderer->m_SwapChain.device);
	UpdateUniformBuffers();

	m_pWRenderer->m_bPrepared = true;
}


void VKRenderer::VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling, bool bUseCubeMap)
{
	//overriden
}


void VKRenderer::CreateUniformBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size,  VkTools::UniformData& uniformData, void *data)
{
	VkResult result  = VkBufferObject::CreateBuffer(m_pWRenderer->m_SwapChain, m_pWRenderer->m_DeviceMemoryProperties, usageFlags, memoryPropertyFlags, size, uniformData, data);
	if (result == VK_SUCCESS)
	{
		uniformData.descriptor.buffer = uniformData.buffer;
		uniformData.descriptor.offset = 0;
		uniformData.descriptor.range = size;
	}
}