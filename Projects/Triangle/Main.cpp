#include <RendererPch\stdafx.h>
#include <External\glm\glm\gtc\matrix_inverse.hpp>

#include "../../Renderer/Vulkan/VkEnums.h"
#include "../../Renderer/Vulkan/VkRenderSystem.h"
#include "../../Renderer/VKRenderer.h"
#include "../../Renderer/Vulkan/VkPipelineManager.h"
#include "../../Renderer/Vulkan/VkRenderPassManager.h"
#include "../../Renderer/Vulkan/VkPipelineLayoutManager.h"
#include "../../Renderer/Vulkan/VkGpuProgram.h"
#include "../../Renderer/Vulkan/VkBufferLayoutManager.h"
#include "../../Renderer/Vulkan/VkBufferObjectManager.h"
#include "../../Renderer/Geometry/VertData.h"

#include "../../Renderer/Vulkan/VkBufferObject.h"
#include "../../Renderer/Vulkan/DrawCallManager.h"
#include "../../Renderer/Vulkan/VkDrawCallDispatcher.h"

#include "../../Renderer/Vulkan/VkGPUMemoryManager.h"
#include "../../Renderer/Vulkan/VkImageManager.h"
#include ".../../Renderer/Vulkan/VkFrameBufferManager.h"

LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void UpdateUniformBuffers();
bool GenerateEvents(MSG& msg);

//Global variables
//====================================================================================
uint32_t	g_iDesktopWidth = 800;
uint32_t	g_iDesktopHeight = 600;
bool		g_bPrepared = false;

glm::vec3	g_Rotation = glm::vec3();
glm::vec3	g_CameraPos = glm::vec3();
glm::vec2	g_MousePos;


// Use to adjust mouse rotation speed
float		g_RotationSpeed = 1.0f;
// Use to adjust mouse zoom speed
float		g_ZoomSpeed = 1.0f;
float       g_zoom = 1.0f;

struct
{
	glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::vec4 viewPos;
	float lodBias = 0.0f;

}m_uboVS;
//====================================================================================

void UpdateUniformBuffers()
{
	// Update matrices
	g_iDesktopWidth = 800;
	g_iDesktopHeight = 600;
	m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)g_iDesktopWidth / (float)g_iDesktopHeight, 0.1f, 256.0f);

	m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, g_zoom));

	m_uboVS.modelMatrix = m_uboVS.viewMatrix * glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_uboVS.viewPos = glm::vec4(0.0f, 0.0f, -5.0f, 0.0f);
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


class RendererTest final : public VKRenderer
{
public:
	RendererTest() {}
	virtual ~RendererTest() {}
};
RendererTest g_Renderer;

/*GLOBALS*/
DOD::Ref render_pass;
std::vector<DOD::Ref> frameBufferRefs;
DOD::Ref draw_call_ref;

void RecreateAfterSwapchainResize()
{
	/*Recreate pipeline layout */
	std::vector<DOD::Ref> pipelineLayoutsToCreate;
	DOD::Ref pipeline_layout_ref = Renderer::Resource::PipelineLayoutManager::CreatePipelineLayout("PipelineLayout1");

	//Describe at which position uniform and image buffer object goes
	auto& descriptorSetLayout = Renderer::Resource::PipelineLayoutManager::GetDescriptorSetLayoutBinding(pipeline_layout_ref);

	pipelineLayoutsToCreate.push_back(pipeline_layout_ref);
	Renderer::Resource::PipelineLayoutManager::CreateResource(pipelineLayoutsToCreate);
	/*======================================================================================================================*/


	/*Recreate render pass*/
	std::vector<DOD::Ref> renderPassesToCreate;
	render_pass = Renderer::Resource::RenderPassManager::CreateRenderPass("RenderPass1");

	Renderer::Resource::RenderPassManager::ResetToDefault(render_pass);

	AttachementDescription sceneAttachment =
	{
		Renderer::Vulkan::RenderSystem::vkColorFormatToUse,
		0u,
		false
	};

	Renderer::Resource::RenderPassManager::GetAttachementDescription(render_pass).push_back(sceneAttachment);

	renderPassesToCreate.push_back(render_pass);
	Renderer::Resource::RenderPassManager::CreateResource(renderPassesToCreate);
	/*======================================================================================================================*/

	/*Recreate frame buffer*/
	frameBufferRefs.clear();
	frameBufferRefs.reserve(Renderer::Vulkan::RenderSystem::vkSwapchainImages.size());

	for (int backBufferIndex = 0; backBufferIndex < Renderer::Vulkan::RenderSystem::vkSwapchainImages.size(); backBufferIndex++)
	{
		DOD::Ref frame_buffer_Ref = Renderer::Resource::FrameBufferManager::CreateFrameBuffer("FrameBuffer1");

		std::string imageName = "Backbuffer" + std::to_string(backBufferIndex);
		DOD::Ref imageRef = Renderer::Resource::ImageManager::GetResourceByName(imageName);

		Renderer::Resource::FrameBufferManager::ResetToDefault(frame_buffer_Ref);
		Renderer::Resource::FrameBufferManager::GetDimensions(frame_buffer_Ref) = Renderer::Vulkan::RenderSystem::backBufferDimensions;
		Renderer::Resource::FrameBufferManager::GetAttachedImiges(frame_buffer_Ref).push_back(imageRef);
		Renderer::Resource::FrameBufferManager::GetRenderPassRef(frame_buffer_Ref) = render_pass;
		Renderer::Resource::FrameBufferManager::CreateResource(frame_buffer_Ref);

		frameBufferRefs.push_back(frame_buffer_Ref);
	}
	/*======================================================================================================================*/

	/*Recreate pipeline*/
	DOD::Ref vert_ref = Renderer::Resource::GpuProgramManager::GetResourceByName("triangle.vert.spv");
	DOD::Ref frag_ref = Renderer::Resource::GpuProgramManager::GetResourceByName("triangle.frag.spv");
	DOD::Ref buffer_layout_ref = Renderer::Resource::BufferLayoutManager::GetResourceByName("BufferLayout1");

	std::vector<DOD::Ref> pipelinesToCreate;

	const DOD::Ref& pipelineRef = Renderer::Resource::PipelineManager::CreatePipeline("Pipeline1");
	Renderer::Resource::PipelineManager::GetVertexShader(pipelineRef) = vert_ref;
	Renderer::Resource::PipelineManager::GetFragmentShader(pipelineRef) = frag_ref;
	Renderer::Resource::PipelineManager::GetRenderPassRef(pipelineRef) = render_pass;
	Renderer::Resource::PipelineManager::GetPipelineLayoutRef(pipelineRef) = pipeline_layout_ref;
	Renderer::Resource::PipelineManager::GetbufferLayoutRef(pipelineRef) = buffer_layout_ref;
	pipelinesToCreate.push_back(pipelineRef);

	Renderer::Resource::PipelineManager::CreateResource(pipelinesToCreate);
	/*====================================================================================================================== */

	/*Recreate draw call*/
	DOD::Ref uniform_buffer_ref = Renderer::Resource::BufferObjectManager::GetResourceByName("UniformBuffer1");
	DOD::Ref staging_buffer_vertices_ref = Renderer::Resource::BufferObjectManager::GetResourceByName("StaggingBufferVertices1");
	DOD::Ref staging_buffer_indices_ref = Renderer::Resource::BufferObjectManager::GetResourceByName("StaggingBufferIndices1");

	//Mesh manager -> Get data from there
	std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2,3,0 };
	uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

	//Link data for drawing
	std::vector<DOD::Ref> drawCallsToCreate;
	draw_call_ref = Renderer::Resource::DrawCallManager::CreateDrawCall("DrawCall1");

	auto& binding_infos = Renderer::Resource::DrawCallManager::GetBindingInfo(draw_call_ref);
	binding_infos.push_back(std::move(Renderer::Resource::BindingInfo{ 0, uniform_buffer_ref }));

	Renderer::Resource::DrawCallManager::GetIndexCount(draw_call_ref) = indexBufferSize;
	Renderer::Resource::DrawCallManager::GetIndexBufferRef(draw_call_ref) = staging_buffer_indices_ref;
	Renderer::Resource::DrawCallManager::GetVertexBufferRef(draw_call_ref) = staging_buffer_vertices_ref;
	Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(draw_call_ref) = pipeline_layout_ref;
	Renderer::Resource::DrawCallManager::GetPipelineRef(draw_call_ref) = pipelineRef;

	drawCallsToCreate.push_back(draw_call_ref);
	Renderer::Resource::DrawCallManager::CreateResource(drawCallsToCreate);
	/*====================================================================================================================== */
}

int main()
{
	std::unique_ptr<VulkanRendererInitializer> renderer_initializer = std::make_unique<VulkanRendererInitializer>();
	renderer_initializer->CreateWindows(g_iDesktopWidth, g_iDesktopHeight, HandleWindowMessages);

	Renderer::Vulkan::RenderSystem::Init(true, true, "Triangle", renderer_initializer->m_hinstance, renderer_initializer->m_HwndWindows);

	//Create GPU Resource
	DOD::Ref vert_ref = Renderer::Resource::GpuProgramManager::CreateGPUProgram("triangle.vert.spv");
	DOD::Ref frag_ref = Renderer::Resource::GpuProgramManager::CreateGPUProgram("triangle.frag.spv");

	//Compile and set to created gpu resource reference
	Renderer::Resource::GpuProgramManager::LoadAndCompileShader(vert_ref, "../../../Assets/Shaders/Triangle/", VK_SHADER_STAGE_VERTEX_BIT);
	Renderer::Resource::GpuProgramManager::LoadAndCompileShader(frag_ref, "../../../Assets/Shaders/Triangle/", VK_SHADER_STAGE_FRAGMENT_BIT);

	//Set bindings positions
	std::vector<DOD::Ref> pipelineLayoutsToCreate;
	DOD::Ref pipeline_layout_ref = Renderer::Resource::PipelineLayoutManager::CreatePipelineLayout("PipelineLayout1");

	//Describe at which position uniform and image buffer object goes
	auto& descriptorSetLayout = Renderer::Resource::PipelineLayoutManager::GetDescriptorSetLayoutBinding(pipeline_layout_ref);
	descriptorSetLayout.push_back(VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0));
	
	pipelineLayoutsToCreate.push_back(pipeline_layout_ref);
	Renderer::Resource::PipelineLayoutManager::CreateResource(pipelineLayoutsToCreate);

	//Create Render Pass
	std::vector<DOD::Ref> renderPassesToCreate;
	render_pass = Renderer::Resource::RenderPassManager::CreateRenderPass("RenderPass1");

	Renderer::Resource::RenderPassManager::ResetToDefault(render_pass);

	AttachementDescription sceneAttachment =
	{
		Renderer::Vulkan::RenderSystem::vkColorFormatToUse,
		0u,
		false
	};

	Renderer::Resource::RenderPassManager::GetAttachementDescription(render_pass).push_back(sceneAttachment);

	renderPassesToCreate.push_back(render_pass);
	Renderer::Resource::RenderPassManager::CreateResource(renderPassesToCreate);

	//Create FrameBuffer
	frameBufferRefs.clear();
	frameBufferRefs.reserve(Renderer::Vulkan::RenderSystem::vkSwapchainImages.size());

	for (int backBufferIndex = 0; backBufferIndex < Renderer::Vulkan::RenderSystem::vkSwapchainImages.size(); backBufferIndex++)
	{
		DOD::Ref frame_buffer_Ref = Renderer::Resource::FrameBufferManager::CreateFrameBuffer("FrameBuffer1");

		std::string imageName = "Backbuffer" + std::to_string(backBufferIndex);
		DOD::Ref imageRef = Renderer::Resource::ImageManager::GetResourceByName(imageName);

		Renderer::Resource::FrameBufferManager::ResetToDefault(frame_buffer_Ref);
		Renderer::Resource::FrameBufferManager::GetDimensions(frame_buffer_Ref) = Renderer::Vulkan::RenderSystem::backBufferDimensions;
		Renderer::Resource::FrameBufferManager::GetAttachedImiges(frame_buffer_Ref).push_back(imageRef);
		Renderer::Resource::FrameBufferManager::GetRenderPassRef(frame_buffer_Ref) = render_pass;
		Renderer::Resource::FrameBufferManager::CreateResource(frame_buffer_Ref);

		frameBufferRefs.push_back(frame_buffer_Ref);
	}


	DOD::Ref buffer_layout_ref = Renderer::Resource::BufferLayoutManager::CreateBufferLayout("BufferLayout1");
	auto& buffer_layout_description = Renderer::Resource::BufferLayoutManager::GetBufferLayoutDescription(buffer_layout_ref);

	//Describe layout of data that will be sent from cpu to gpu
	buffer_layout_description = 
	{ 
		{0, Renderer::Resource::BufferObjectType::VERTEX, VK_FORMAT_R32G32B32_SFLOAT} ,
		{1, Renderer::Resource::BufferObjectType::NORMAL, VK_FORMAT_R32G32B32_SFLOAT } ,
		{2, Renderer::Resource::BufferObjectType::TEX, VK_FORMAT_R32G32B32_SFLOAT } 
	};

	Renderer::Resource::BufferLayoutManager::CreateResource(buffer_layout_ref); 

	//Create pipeline
	std::vector<DOD::Ref> pipelinesToCreate;

	const DOD::Ref& pipelineRef = Renderer::Resource::PipelineManager::CreatePipeline("Pipeline1");
	Renderer::Resource::PipelineManager::GetVertexShader(pipelineRef) = vert_ref;
	Renderer::Resource::PipelineManager::GetFragmentShader(pipelineRef) = frag_ref;
	Renderer::Resource::PipelineManager::GetRenderPassRef(pipelineRef) = render_pass;
	Renderer::Resource::PipelineManager::GetPipelineLayoutRef(pipelineRef) = pipeline_layout_ref;
	Renderer::Resource::PipelineManager::GetbufferLayoutRef(pipelineRef) = buffer_layout_ref;
	pipelinesToCreate.push_back(pipelineRef);

	Renderer::Resource::PipelineManager::CreateResource(pipelinesToCreate);

	//SENDING DATA TO GPU
	//{
		//Get data to buffer
		struct Vertex 
		{
			float pos[3];
			float uv[2];
			float normal[3];
		};

#define DIM 1.0f
#define NORMAL { 0.0f, 0.0f, 1.0f }
		drawVert vertData[4] =
		{
			{ glm::vec3(DIM,DIM,    0),	   glm::vec3(0,0,1), glm::vec2(1,1) },
			{ glm::vec3(-DIM,DIM,  0),    glm::vec3(0,0,1), glm::vec2(0,1) },
			{ glm::vec3(-DIM, -DIM,  0),    glm::vec3(0,0,1), glm::vec2(0,0) },
			{ glm::vec3(DIM,-DIM,  0),    glm::vec3(0,0,1), glm::vec2(1,0) },
		};

		// Setup indices
		std::vector<uint32_t> indexBuffer = { 0, 1, 2, 2,3,0 };
		uint32_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

		//Update uniform buffer
		UpdateUniformBuffers();

		DOD::Ref staging_buffer_vertices_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("StaggingBufferVertices1");
		
		Renderer::Resource::BufferObjectManager::GetBufferSize(staging_buffer_vertices_ref) = static_cast<uint32_t>(sizeof(drawVert) * 4);
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(staging_buffer_vertices_ref) = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(staging_buffer_vertices_ref) = vertData;
		Renderer::Resource::BufferObjectManager::CreateResource(staging_buffer_vertices_ref, Renderer::Vulkan::RenderSystem::vkPhysicalDeviceMemoryProperties);

		DOD::Ref staging_buffer_indices_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("StaggingBufferIndices1");

		Renderer::Resource::BufferObjectManager::GetBufferSize(staging_buffer_indices_ref) = indexBufferSize;
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(staging_buffer_indices_ref) = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(staging_buffer_indices_ref) = indexBuffer.data();
		Renderer::Resource::BufferObjectManager::CreateResource(staging_buffer_indices_ref, Renderer::Vulkan::RenderSystem::vkPhysicalDeviceMemoryProperties);

		DOD::Ref uniform_buffer_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("UniformBuffer1");

		Renderer::Resource::BufferObjectManager::GetBufferSize(uniform_buffer_ref) = sizeof(m_uboVS);
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(uniform_buffer_ref) = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(uniform_buffer_ref) = &m_uboVS;
		Renderer::Resource::BufferObjectManager::CreateResource(uniform_buffer_ref, Renderer::Vulkan::RenderSystem::vkPhysicalDeviceMemoryProperties);
	//}

	//Link data for drawing
	std::vector<DOD::Ref> drawCallsToCreate;
	draw_call_ref = Renderer::Resource::DrawCallManager::CreateDrawCall("DrawCall1");

	auto& binding_infos = Renderer::Resource::DrawCallManager::GetBindingInfo(draw_call_ref);
	binding_infos.push_back(std::move(Renderer::Resource::BindingInfo{ 0, uniform_buffer_ref }));

	Renderer::Resource::DrawCallManager::GetIndexCount(draw_call_ref) = indexBufferSize;
	Renderer::Resource::DrawCallManager::GetIndexBufferRef(draw_call_ref) = staging_buffer_indices_ref;
	Renderer::Resource::DrawCallManager::GetVertexBufferRef(draw_call_ref) = staging_buffer_vertices_ref;
	Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(draw_call_ref) = pipeline_layout_ref;
	Renderer::Resource::DrawCallManager::GetPipelineRef(draw_call_ref) = pipelineRef;

	drawCallsToCreate.push_back(draw_call_ref);
	Renderer::Resource::DrawCallManager::CreateResource(drawCallsToCreate);

#if defined(_WIN32)
	MSG msg;
#endif

	VkClearValue clearValues[2];
	clearValues[0].color = { { 0.5f, 0.5f, 0.5f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();
		Renderer::Vulkan::RenderSystem::StartFrame();
		
		//Generate command buffers
		Renderer::Vulkan::RenderSystem::BeginRenderPass(render_pass, frameBufferRefs[Renderer::Vulkan::RenderSystem::backBufferIndex], VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS, 2, clearValues);
		Renderer::Vulkan::DrawCall::QueuDrawCall(draw_call_ref, frameBufferRefs[Renderer::Vulkan::RenderSystem::backBufferIndex], render_pass, g_iDesktopWidth, g_iDesktopHeight);
		Renderer::Vulkan::RenderSystem::EndRenderPass();

		Renderer::Vulkan::RenderSystem::EndFrame();
		auto tEnd = std::chrono::high_resolution_clock::now();
	}

	Renderer::Vulkan::RenderSystem::DestroyCommandBuffers();

	//Release resources
	Renderer::Resource::DrawCallManager::DestroyResources(Renderer::Resource::DrawCallManager::activeRefs);
	Renderer::Resource::RenderPassManager::DestroyResources(Renderer::Resource::RenderPassManager::activeRefs);
	Renderer::Resource::GpuProgramManager::DestroyResources(Renderer::Resource::GpuProgramManager::activeRefs);
	Renderer::Resource::PipelineLayoutManager::DestroyResources(Renderer::Resource::PipelineLayoutManager::activeRefs);
	Renderer::Resource::PipelineManager::DestroyResources(Renderer::Resource::PipelineManager::activeRefs);
	Renderer::Resource::BufferObjectManager::DestroyResources(Renderer::Resource::BufferObjectManager::activeRefs);
	Renderer::Vulkan::GpuMemoryManager::Destroy();

	//Will fix later...
	Renderer::Vulkan::RenderSystem::DestroyVulkanDebug(true);

	system("pause");
	return 0;
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

	Renderer::Vulkan::RenderSystem::backBufferDimensions = glm::uvec2(g_iDesktopWidth, g_iDesktopHeight);
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
	case WM_SIZE:
		RECT rect;
		if (GetWindowRect(hWnd, &rect))
		{
			WIN_Sizing(wParam, &rect);
		}
		break;
	case WM_EXITSIZEMOVE:
			Renderer::Vulkan::RenderSystem::ResizeSwapchain();
			RecreateAfterSwapchainResize();
			//g_Renderer.VWindowResize(g_iDesktopHeight, g_iDesktopWidth);
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}