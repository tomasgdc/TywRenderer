#include <iostream>
#include "../../Renderer/VKRenderer.h"

#include "../../Renderer/Vulkan/VkPipelineManager.h"
#include "../../Renderer/Vulkan/VkRenderPassManager.h"
#include "../../Renderer/Vulkan/VkPipelineLayoutManager.h"
#include "../../Renderer/Vulkan/VkGpuProgram.h"

LRESULT CALLBACK HandleWindowMessages(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int g_iDesktopWidth = 800;
int g_iDesktopHeight = 600;
bool g_bPrepared = false;
VKRenderer g_Renderer;

int main()
{
	g_bPrepared = g_Renderer.VInitRenderer(800, 600, false, HandleWindowMessages);

	DOD::Ref ref(0, 0);

	Renderer::Resource::RenderPassManager::CreateRenderPass(ref);
	Renderer::Resource::PipelineLayoutManager::CreatePipelineLayout(ref);
	
	Renderer::Resource::GpuProgramManager::LoadShader(ref, "../../../Assets/Shaders/Triangle/triangle.vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
	Renderer::Resource::GpuProgramManager::LoadShader(ref, "../../../Assets/Shaders/Triangle/triangle.frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

	Renderer::Resource::PipelineManager::CreateGraphicsPipelineData(ref);
	

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