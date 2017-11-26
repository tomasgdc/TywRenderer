#include <RendererPch\stdafx.h>
#include <External\glm\glm\gtc\matrix_inverse.hpp>

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

LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

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

class RendererTest final : public VKRenderer
{
private:

	// Synchronization semaphores
	// Semaphores are used to synchronize dependencies between command buffers
	// We use them to ensure that we e.g. don't present to the swap chain
	// until all rendering has completed
	struct 
	{
		// Swap chain image presentation
		VkSemaphore presentComplete;
		// Command buffer submission and execution
		VkSemaphore renderComplete;
	} Semaphores;


public:

	struct
	{
		glm::mat4 projectionMatrix;
		glm::mat4 modelMatrix;
		glm::mat4 viewMatrix;
		glm::vec4 viewPos;
		float lodBias = 0.0f;

	}m_uboVS;


	RendererTest() {}
	~RendererTest() {}

	void StartFrame() override;
	void PrepareSemaphore() override;
	void UpdateUniformBuffers() override;
};
RendererTest g_Renderer;




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

int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(800, 600, false, HandleWindowMessages);
	g_Renderer.UpdateUniformBuffers();

	DOD::Ref ref(0, 0);

	//Allocate memory
	Renderer::Resource::RenderPassManager::init();
	Renderer::Resource::PipelineLayoutManager::init();
	Renderer::Resource::GpuProgramManager::init();
	Renderer::Resource::PipelineManager::init();
	Renderer::Resource::BufferLayoutManager::init();
	Renderer::Resource::BufferObjectManager::init();
	Renderer::Resource::DrawCallManager::init();
	
	//Create GPU Resource
	DOD::Ref vert_ref = Renderer::Resource::GpuProgramManager::CreateGPUProgram("triangle.vert.spv");
	DOD::Ref frag_ref = Renderer::Resource::GpuProgramManager::CreateGPUProgram("triangle.frag.spv");

	//Compile and set to created gpu resource reference
	Renderer::Resource::GpuProgramManager::LoadAndCompileShader(vert_ref, "../../../Assets/Shaders/Triangle/", VK_SHADER_STAGE_VERTEX_BIT);
	Renderer::Resource::GpuProgramManager::LoadAndCompileShader(frag_ref, "../../../Assets/Shaders/Triangle/", VK_SHADER_STAGE_FRAGMENT_BIT);

	//Set bindings positions
	DOD::Ref pipeline_layout_ref = Renderer::Resource::PipelineLayoutManager::CreatePipelineLayout("Pipeline_layout_1");

	//Describe at which position uniform and image buffer object goes
	auto& descriptor_layout = Renderer::Resource::PipelineLayoutManager::GetDescriptorSetLayoutBinding(pipeline_layout_ref);
	descriptor_layout.push_back(VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0));
	//descriptor_layout.push_back(VkTools::Initializer::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1));

	Renderer::Resource::PipelineLayoutManager::CreateResource(pipeline_layout_ref);

	//Connect shader to pipeline
	Renderer::Resource::PipelineManager::GetVertexShader(ref) = vert_ref;
	Renderer::Resource::PipelineManager::GetFragmentShader(ref) = frag_ref;

	//Create Render Pass
	DOD::Ref render_pass = Renderer::Resource::RenderPassManager::CreateRenderPass("Pass_1");
	Renderer::Resource::RenderPassManager::CreateResource(render_pass);


	DOD::Ref buffer_layout_ref = Renderer::Resource::BufferLayoutManager::CreateBufferLayout("Buffer_Layout_1");
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
	DOD::Ref pipeline_ref = Renderer::Resource::PipelineManager::CreatePipeline("Pipeline_1");
	Renderer::Resource::PipelineManager::CreateResource(pipeline_ref);

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


		DOD::Ref staging_buffer_vertices_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("StaggingBuffer_Vertices_1");
		
		Renderer::Resource::BufferObjectManager::GetBufferSize(staging_buffer_vertices_ref) = static_cast<uint32_t>(sizeof(drawVert) * 4);
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(staging_buffer_vertices_ref) = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(staging_buffer_vertices_ref) = vertData;
		Renderer::Resource::BufferObjectManager::CreateResource(staging_buffer_vertices_ref, g_Renderer.m_pWRenderer->m_DeviceMemoryProperties);

		DOD::Ref staging_buffer_indices_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("StaggingBuffer_Indices_1");

		Renderer::Resource::BufferObjectManager::GetBufferSize(staging_buffer_indices_ref) = indexBufferSize;
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(staging_buffer_indices_ref) = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(staging_buffer_indices_ref) = indexBuffer.data();
		Renderer::Resource::BufferObjectManager::CreateResource(staging_buffer_indices_ref, g_Renderer.m_pWRenderer->m_DeviceMemoryProperties);

		DOD::Ref uniform_buffer_ref = Renderer::Resource::BufferObjectManager::CreateBufferOjbect("Uniform_Buffer_1");

		Renderer::Resource::BufferObjectManager::GetBufferSize(uniform_buffer_ref) = sizeof(g_Renderer.m_uboVS);
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(uniform_buffer_ref) = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(uniform_buffer_ref) = &g_Renderer.m_uboVS;
		Renderer::Resource::BufferObjectManager::CreateResource(uniform_buffer_ref, g_Renderer.m_pWRenderer->m_DeviceMemoryProperties);
	//}

	//Link data for drawing
	DOD::Ref draw_call_ref = Renderer::Resource::DrawCallManager::CreateDrawCall("Draw_call_1");
	auto& binding_infos = Renderer::Resource::DrawCallManager::GetBindingInfo(draw_call_ref);

	binding_infos.push_back(std::move(Renderer::Resource::BindingInfo{ 0, uniform_buffer_ref }));

	Renderer::Resource::DrawCallManager::GetIndexCount(draw_call_ref) = indexBufferSize;
	Renderer::Resource::DrawCallManager::GetIndexBufferRef(draw_call_ref) = staging_buffer_indices_ref;
	Renderer::Resource::DrawCallManager::GetVertexBufferRef(draw_call_ref) = staging_buffer_vertices_ref;
	Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(draw_call_ref) = pipeline_layout_ref;
	Renderer::Resource::DrawCallManager::GetPipelineRef(draw_call_ref) = pipeline_ref;

	Renderer::Resource::DrawCallManager::CreateResource(draw_call_ref);
	
	//Build commands
	Renderer::Vulkan::DrawCall::BuildCommandBuffer(draw_call_ref, g_iDesktopWidth, g_iDesktopHeight);

#if defined(_WIN32)
	MSG msg;
#endif

	while (TRUE)
	{
		if (!GenerateEvents(msg))break;

		auto tStart = std::chrono::high_resolution_clock::now();
		g_Renderer.StartFrame();
		//Do something
		g_Renderer.EndFrame(nullptr);
		auto tEnd = std::chrono::high_resolution_clock::now();
	}

	//Release resources
	Renderer::Resource::RenderPassManager::DestroyResources(Renderer::Resource::RenderPassManager::activeRefs);
	Renderer::Resource::GpuProgramManager::DestroyResources(Renderer::Resource::GpuProgramManager::activeRefs);
	Renderer::Resource::PipelineLayoutManager::DestroyResources(Renderer::Resource::PipelineLayoutManager::activeRefs);
	Renderer::Resource::PipelineManager::DestroyResources(Renderer::Resource::PipelineManager::activeRefs);
	Renderer::Resource::BufferObjectManager::DestroyResources(Renderer::Resource::BufferObjectManager::activeRefs);
	Renderer::Resource::DrawCallManager::DestroyResources(Renderer::Resource::DrawCallManager::activeRefs);

	system("pause");
	return 0;
}

void RendererTest::UpdateUniformBuffers()
{
	// Update matrices
	m_uboVS.projectionMatrix = glm::perspective(glm::radians(60.0f), (float)m_WindowWidth / (float)m_WindowHeight, 0.1f, 256.0f);

	m_uboVS.viewMatrix = glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, g_zoom));

	m_uboVS.modelMatrix = m_uboVS.viewMatrix * glm::translate(glm::mat4(), glm::vec3(0.0f, 0.0f, -5.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	m_uboVS.modelMatrix = glm::rotate(m_uboVS.modelMatrix, glm::radians(g_Rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

	m_uboVS.viewPos = glm::vec4(0.0f, 0.0f, -5.0f, 0.0f);

	//Renderer::Resource::BufferObjectManager::

	// Map uniform buffer and update it
	uint8_t *pData;
	//VK_CHECK_RESULT(vkMapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory, 0, sizeof(m_uboVS), 0, (void **)&pData));
	//memcpy(pData, &m_uboVS, sizeof(m_uboVS));
	//vkUnmapMemory(m_pWRenderer->m_SwapChain.device, uniformDataVS.memory);
}

void RendererTest::PrepareSemaphore()
{
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = NULL;

	// This semaphore ensures that the image is complete
	// before starting to submit again
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.presentComplete));

	// This semaphore ensures that all commands submitted
	// have been finished before submitting the image to the queue
	VK_CHECK_RESULT(vkCreateSemaphore(m_pWRenderer->m_SwapChain.device, &semaphoreCreateInfo, nullptr, &Semaphores.renderComplete));
}

void RendererTest::StartFrame()
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
		// The submit infor strcuture contains a list of
		// command buffers and semaphores to be submitted to a queue
		// If you want to submit multiple command buffers, pass an array
		VkPipelineStageFlags pipelineStages = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
		VkSubmitInfo submitInfo = VkTools::Initializer::SubmitInfo();
		submitInfo.pWaitDstStageMask = &pipelineStages;
		// The wait semaphore ensures that the image is presented 
		// before we start submitting command buffers agein
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &Semaphores.presentComplete;
		// Submit the currently active command buffer
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &m_pWRenderer->m_DrawCmdBuffers[m_pWRenderer->m_currentBuffer];
		// The signal semaphore is used during queue presentation
		// to ensure that the image is not rendered before all
		// commands have been submitted
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &Semaphores.renderComplete;

		// Submit to the graphics queue
		VK_CHECK_RESULT(vkQueueSubmit(m_pWRenderer->m_Queue, 1, &submitInfo, VK_NULL_HANDLE));

		// Present the current buffer to the swap chain
		// We pass the signal semaphore from the submit info
		// to ensure that the image is not rendered until
		// all commands have been submitted
		VK_CHECK_RESULT(m_pWRenderer->m_SwapChain.QueuePresent(m_pWRenderer->m_Queue, m_pWRenderer->m_currentBuffer, Semaphores.renderComplete));
	}
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
		if (g_bPrepared) {
			g_Renderer.VWindowResize(g_iDesktopHeight, g_iDesktopWidth);
		}
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}