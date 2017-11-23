#include <iostream>
#include <External\glm\glm\glm.hpp>

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

LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int g_iDesktopWidth = 800;
int g_iDesktopHeight = 600;
bool g_bPrepared = false;
VKRenderer g_Renderer;

struct 
{
	glm::mat4 projectionMatrix;
	glm::mat4 modelMatrix;
	glm::mat4 viewMatrix;
	glm::vec4 viewPos;
	float lodBias = 0.0f;

}m_uboVS;


int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(800, 600, false, HandleWindowMessages);

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
	DOD::Ref pipeline_layout_ref = Renderer::Resource::PipelineLayoutManager::CreatePipelineLayout("Pipeline_1");

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

		Renderer::Resource::BufferObjectManager::GetBufferSize(uniform_buffer_ref) = sizeof(m_uboVS);
		Renderer::Resource::BufferObjectManager::GetBufferUsageFlag(uniform_buffer_ref) = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		Renderer::Resource::BufferObjectManager::GetBufferData(uniform_buffer_ref) = &m_uboVS;
		Renderer::Resource::BufferObjectManager::CreateResource(uniform_buffer_ref, g_Renderer.m_pWRenderer->m_DeviceMemoryProperties);
	//}

	//Link data for drawing
	DOD::Ref draw_call_ref = Renderer::Resource::DrawCallManager::CreateDrawCall("Draw_call_1");
	auto& binding_infos = Renderer::Resource::DrawCallManager::GetBindingInfo(draw_call_ref);

	binding_infos.push_back(std::move(Renderer::Resource::BindingInfo{ 0, uniform_buffer_ref }));

	Renderer::Resource::DrawCallManager::GetPipelineLayoutRef(draw_call_ref) = pipeline_layout_ref;
	Renderer::Resource::DrawCallManager::CreateResource(draw_call_ref);
	
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