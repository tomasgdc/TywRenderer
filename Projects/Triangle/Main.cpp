/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/

#include <RendererPch\stdafx.h>
#include <Renderer\TywRenderer.h>
#include <Renderer\TywRendererHelpers.h>


//Triangle Includes
#include "Main.h"
#include <conio.h>

//Renderer Includes
#include <Renderer\VKRenderer.h>
#include <Renderer\Vulkan\VulkanTools.h>


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
	// The pipeline (state objects) is a static store for the 3D pipeline states (including shaders)
	// Other than OpenGL this makes you setup the render states up-front
	// If different render states are required you need to setup multiple pipelines
	// and switch between them
	// Note that there are a few dynamic states (scissor, viewport, line width) that
	// can be set from a command buffer and does not have to be part of the pipeline
	// This basic example only uses one pipeline
	VkPipeline pipeline;

	// The pipeline layout defines the resource binding slots to be used with a pipeline
	// This includes bindings for buffes (ubos, ssbos), images and sampler
	// A pipeline layout can be used for multiple pipeline (state objects) as long as 
	// their shaders use the same binding layout
	VkPipelineLayout pipelineLayout;

	// The descriptor set stores the resources bound to the binding points in a shader
	// It connects the binding points of the different shaders with the buffers and images
	// used for those bindings
	VkDescriptorSet descriptorSet;

	// The descriptor set layout describes the shader binding points without referencing
	// the actual buffers. 
	// Like the pipeline layout it's pretty much a blueprint and can be used with
	// different descriptor sets as long as the binding points (and shaders) match
	VkDescriptorSetLayout descriptorSetLayout;

	// Synchronization semaphores
	// Semaphores are used to synchronize dependencies between command buffers
	// We use them to ensure that we e.g. don't present to the swap chain
	// until all rendering has completed
	struct {
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
		// Text overlay submission and execution
		VkSemaphore textOverlayComplete;
	} Semaphores;


	struct {
		VkBuffer buf;
		VkDeviceMemory mem;
		VkPipelineVertexInputStateCreateInfo inputState;
		std::vector<VkVertexInputBindingDescription> bindingDescriptions;
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
	} vertices;

	struct {
		int count;
		VkBuffer buf;
		VkDeviceMemory mem;
	} indices;

	struct {
		VkBuffer buffer;
		VkDeviceMemory memory;
		VkDescriptorBufferInfo descriptor;
	}  uniformDataVS;


	// For simplicity we use the same uniform block layout as in the shader:
	//
	//	layout(set = 0, binding = 0) uniform UBO
	//	{
	//		mat4 projectionMatrix;
	//		mat4 modelMatrix;
	//		mat4 viewMatrix;
	//	} ubo;
	//
	// This way we can just memcopy the ubo data to the ubo
	// Note that you should be using data types that align with the GPU
	// in order to avoid manual padding
	struct {
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
	} uboVS;
public:
	Renderer();
	~Renderer();
	void BuildCommandBuffers() override;
	void UpdateUniformBuffers() override;
	void PrepareUniformBuffers() override;
	void PrepareVertices(bool useStagingBuffers) override;
	void StartFrame() override;
	void PreparePipeline() override;
	void SetupDescriptorSetLayout() override;
	void SetupDescriptorPool() override;
	void SetupDescriptorSet() override;
	void PrepareSemaphore() override;
};

Renderer::Renderer()
{

}


Renderer::~Renderer()
{
}

void Renderer::PrepareSemaphore()
{

}


void Renderer::SetupDescriptorSet()
{
}


void Renderer::SetupDescriptorPool()
{

}


void Renderer::SetupDescriptorSetLayout()
{

}


void Renderer::PreparePipeline()
{
	vkfx::RenderPassHandle renderpass;
	vkfx::PipelineLayoutHandle pipelinelayouthandlepass;

	//Get pipelineCreateInfoHandle
	vkfx::PipelineCreateInfoHandle pipelineCreateInfo = vkfx::CreatePipelineCreateInfoHandle(pipelinelayouthandlepass, renderpass, 0);

	//Store information into pipelineCreateInfoHandle which will tell how pipeline should act.
	vkfx::SetPipelineInputAssemblyStateCreateInfo(pipelineCreateInfo, VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
	vkfx::SetPipelineRasterizationStateCreateInfo(pipelineCreateInfo, VK_POLYGON_MODE_FILL, VK_CULL_MODE_BACK_BIT, VK_FRONT_FACE_CLOCKWISE, 0);
	void * pBlendAttachement = vkfx::GetPipelineColorBlendAttachmentState(0xf, VK_FALSE);
	vkfx::SetPipelineColorBlendStateCreateInfo(pipelineCreateInfo, 1, pBlendAttachement);
	vkfx::SetPipelineDepthStencilStateCreateInfo(pipelineCreateInfo, VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS_OR_EQUAL);
	vkfx::SetPipelineViewportStateCreateInfo(pipelineCreateInfo, 1, 1, 0);
	vkfx::SetPipelineMultisampleStateCreateInfo(pipelineCreateInfo, VK_SAMPLE_COUNT_1_BIT,0);

	std::vector<VkDynamicState> dynamicStateEnables = 
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	vkfx::SetPipelineDynamicStateCreateInfo(pipelineCreateInfo, dynamicStateEnables.data(),dynamicStateEnables.size(), 0);

	//Get Shaders
	std::array<vkfx::ShaderStageHandle, 2> shaderStages =
	{
		vkfx::CreateShaderStageHandle(GetAssetPath() + "Shaders/SSAO/Fullscreen.vert.spv", VK_SHADER_STAGE_VERTEX_BIT),
		vkfx::CreateShaderStageHandle(GetAssetPath() + "Shaders/SSAO/DefferedModel.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT)
	};
	vkfx::SetShaderStages(pipelineCreateInfo, shaderStages.data(), shaderStages.size());

	//Pass PipelineCreateInfoHandle in order to get PipelineHandle
	vkfx::PipelineHandle pipelineHandle = vkfx::CreatePipelineHandle(&pipelineCreateInfo, 1);
}

void Renderer::BuildCommandBuffers()
{

	
}

void Renderer::UpdateUniformBuffers()
{
	// Update matrices
	uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 256.0f);

	uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));

	uboVS.modelMatrix = glm::mat4();
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	uboVS.modelMatrix = glm::rotate(uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	// Map uniform buffer and update it
	uint8_t *pData;
	VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(uboVS), 0, (void **)&pData));
	memcpy(pData, &uboVS, sizeof(uboVS));
	vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory);
}

void Renderer::PrepareUniformBuffers()
{
	vkfx::UniformDataHandle uboHandle = vkfx::CreateUniformBuffer(&uboVS, sizeof(uboVS));

	UpdateUniformBuffers();
}

void Renderer::PrepareVertices(bool useStagingBuffers)
{
	struct Vertex 
	{
		float pos[3];
		float col[3];
	};

	// Setup vertices
	std::vector<Vertex> vertexBuffer = {
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 0.0f, 0.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f, 0.0f } },
		{ { 0.0f, -1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};

	uint32_t vertexBufferSize = static_cast<uint32_t>(vertexBuffer.size()) * sizeof(Vertex);

	// Setup indices
	std::vector<uint32_t> indexBuffer = { 0, 1, 2 };
	indices.count = static_cast<uint32_t>(indexBuffer.size());
	uint32_t indexBufferSize = indices.count * sizeof(uint32_t);


}

void Renderer::StartFrame()
{
	
}

int main()
{

	g_bPrepared = vkfx::initialize(800, 600, false, HandleWindowMessages);

#if defined(_WIN32)
	MSG msg;
#endif

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;
		vkfx::frame();
	}
	vkfx::shutdown();

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
		g_ZoomSpeed += (float)wheelDelta * 0.005f * g_ZoomSpeed;
		//camera.translate(glm::vec3(0.0f, 0.0f, (float)wheelDelta * 0.005f * zoomSpeed))
		//g_Renderer.UpdateUniformBuffers();
		break;
	}
	case WM_MOUSEMOVE:
		if (wParam & MK_RBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_ZoomSpeed += (g_MousePos.y - (float)posy) * .005f * g_ZoomSpeed;
			//camera.translate(glm::vec3(-0.0f, 0.0f, (mousePos.y - (float)posy) * .005f * zoomSpeed));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			//g_Renderer.UpdateUniformBuffers();
		}
		if (wParam & MK_LBUTTON)
		{
			int32_t posx = LOWORD(lParam);
			int32_t posy = HIWORD(lParam);
			g_Rotation.x += (g_MousePos.y - (float)posy) * 1.25f * g_RotationSpeed;
			g_Rotation.y -= (g_MousePos.x - (float)posx) * 1.25f * g_RotationSpeed;
			//camera.rotate(glm::vec3((mousePos.y - (float)posy), -(mousePos.x - (float)posx), 0.0f));
			g_MousePos = glm::vec2((float)posx, (float)posy);
			//g_Renderer.UpdateUniformBuffers();
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
			vkfx::windowResize(g_iDesktopHeight, g_iDesktopWidth);
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