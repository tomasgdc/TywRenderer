/*
*	Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
*	GitHub repository - https://github.com/TywyllSoftware/TywRenderer
*	This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
*/
#pragma once
#define IMGUI_INCLUDE_IMCONFIG_H

//forward declared
namespace VkTools
{
	class VulkanTextureLoader;
}

struct srfTriangles_t;
struct glconfig_st;
struct gl_params;
class  ImageManager;
class VkFont;


#include <External\vulkan\vulkan.h>
#include "IRenderer.h"




struct glconfig_st
{
	const char *		renderer_string;
	const char *		vendor_string;
	const char *		version_string;
	const char *		extensions_string;
	const char *		wgl_extensions_string;
	const char *		shading_language_string;

	float				fGLVersion;
	float				fMaxTextureAnisotropy;

	uint32_t			iNumbOfSuppExt;			//number of supported extensions
	uint32_t			iMaxTextureSize;			// queried from GL
	uint32_t			iMaxTextureCoords;
	uint32_t			iMaxTextureImageUnits;
	uint32_t			iUniformBufferOffsetAlignment;
	
	uint32_t			iColorBits;
	uint32_t			iDepthBits;
	uint32_t			iStencilBits;

	uint32_t			iNativeScreenWidth; // this is the native screen width resolution of the renderer
	uint32_t			iNativeScreenHeight; // this is the native screen height resolution of the renderer
	uint32_t			iDisplayFrequency;
	uint32_t			iMultiSamples;
	uint32_t			iNumbMonitor;	// monitor number

	bool				bIsStereoPixelFormat;
	bool				bStereoPixelFormatAvailable;
	bool				bSyncAvailable;
	bool				bSwapControlTearAvailable;		//adaptive vsync
	bool				bTimerQueryAvailable;
	bool				bIsFullScreen;
};

#pragma pack(push, 1)
struct gl_params
{
	uint32_t		iXCoord; //screen position in x
	uint32_t		iYCoord; //screen position in y
	uint32_t		iWidth;
	uint32_t		iHeight;
	uint32_t		iDisplayHz;

	// -1 = borderless window for spanning multiple display
	// 0 = windowed, otherwise 1 based monitor number to go full screen on
	uint16_t		iFullScreen;		
	uint16_t		iMultiSamples;
	uint16_t		iSwapInterval;	// 0 - no vsync | 1 - vsync on | 2 - adaptive vsync
	bool			bStereo;
};
#pragma pack(pop)

#include "VulkanRendererInitializer.h"

class TYWRENDERER_API VKRenderer: public IRenderer
{
public:
	VKRenderer();
	virtual ~VKRenderer();

	/*
		Cleans all resources used by VKRenderer
	*/
	void VShutdown() override;


	/*
		Initializes Vulkan Renderer and Windows

		@param: uint32_t height
		@param: uint32_t widht
		@param: bool isFullscreen
		@param: LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM)
	*/
	bool VInitRenderer(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM)) override;


	/*
		Used for creating ufnirom buffer object

		@parma: VkBufferUsageFlags usageFlags
		@param: VkMemoryPropertyFlags memoryPropertyFlags
		@param: VkDeviceSize size
		@param: VkTools::UniformData& uniformData
		@param: void *data
	*/
	void CreateUniformBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags, VkDeviceSize size, VkTools::UniformData& uniformData, void *data);

	inline void VSetBackgroundColor(BYTE bgA, BYTE bgR, BYTE bgG, BYTE bgB)
	{
		m_backgroundColor[0] = static_cast<float>(bgA) / 255.0f;
		m_backgroundColor[1] = static_cast<float>(bgR) / 255.0f;
		m_backgroundColor[2] = static_cast<float>(bgG) / 255.0f;
		m_backgroundColor[3] = static_cast<float>(bgB) / 255.0f;
	}

	/*
		Resizes the main swapchain, recreates command buffers and update uniform buffers.
		TODO: Abstarct better for proper all created new command buffers recreation

		@param: uint32_t iHeight
		@parma: uint32_t iWidth
	*/
	void VWindowResize(uint32_t iHeight, uint32_t iWidth) override;

	bool VPreRender() override;
	bool VPostRender() override;
	HRESULT VOnRestore() override;


	std::shared_ptr<IRenderState> VPrepareAlphaPass() override;
	std::shared_ptr<IRenderState> VPrepareSkyBoxPass() override;

	//Creates files for logging
	void VSetLogPath();

	virtual void StartFrame();
	virtual void EndFrame(uint64_t* gpuMicroSec);
public:
	//Log Renderer
	void   Logv(const char* format, ...);

	//Log Rendering Streaming
	void   LogStrv(char* format, ...);

	//Log Renderer Shaders
	void   LogShv(char* format, ...);

	//Prints directly log info to screen
	void   Log(char* str,...);

	static std::string GetAssetPath() { return "../../../Assets/"; }
public:
	//Calculates swap command and calculates how much time gpu took to process requests
	void SwapCommandBuffers_FinnishRendering(uint64_t* gpuMicroSec);

	// Create synchronzation semaphores
	virtual void PrepareSemaphore();


	//ovverridable
	virtual void PreparePipeline();

	//Loads Vulkan Shaders
	VkPipelineShaderStageCreateInfo LoadShader(std::string fileName, VkShaderStageFlagBits stage);

	//overridable
	virtual void LoadAssets();

	//ovveridable
	virtual void SetupDescriptorSetLayout();

	//ovveridable
	virtual void SetupDescriptorPool();

	//ovveridable
	virtual void BuildCommandBuffers();

	//ovveridable
	virtual void UpdateUniformBuffers();

	//ovveridable
	virtual void PrepareUniformBuffers();

	//ovveridable
	virtual void PrepareVertices(bool useStagingBuffers);

	//Ovveridable texutre loader
	virtual void VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling, bool bUseCubeMap = false);


	VkCommandBuffer GetCommandBuffer(bool begin);

	void FlushCommandBuffer(VkCommandBuffer commandBuffer);

	//overridable
	virtual void SetupDescriptorSet();

	//overridable
	virtual void LoadGUI();

protected:
	//Log Renderer
	FILE*                     m_LogFile;

	//Log Rendering Streaming
	FILE*                     m_LogFileStr;

	//Log Renderer Shaders
	FILE*                     m_LogFileSh;

protected:
	bool								m_bIsOpenglRunning;
	bool								m_bLogRenderer;
	uint32_t							m_iTimerQueryId;
	float								m_backgroundColor[4];

	glconfig_st							m_glConfigs;
	gl_params							m_glParams;

	VulkanRendererInitializer			*m_pWRenderer;
	VkTools::VulkanTextureLoader		*m_pTextureLoader;
	ImageManager						*m_pImageManager;

protected:
	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> m_ShaderModules;

	VkClearColorValue defaultClearColor = { { 0.5f, 0.5f, 0.5f, 1.0f } };

	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;
	bool paused = false;


	uint32_t m_WindowWidth;
	uint32_t m_WindowHeight;
public:
	// fps timer (one second interval)
	float fpsTimer = 0.0f;
	float frameTimer = 1.0f;
	// Frame counter to display fps
	uint32_t frameCounter = 0;
	uint32_t lastFPS = 0;
};



/*
============================================================
TR_TRISURF
============================================================
*/

srfTriangles_t *	R_AllocStaticTriSurf();
void				R_AllocStaticTriSurfVerts(srfTriangles_t *tri, int numVerts);
void				R_AllocStaticTriSurfIndexes(srfTriangles_t *tri, int numIndexes);

void				R_FreeStaticTriSurfSilIndexes(srfTriangles_t *tri);
void				R_FreeStaticTriSurf(srfTriangles_t *tri);
void				R_FreeStaticTriSurfVerts(srfTriangles_t *tri);

void				R_CreateStaticBuffersForTri(srfTriangles_t & tri);
//=============================================




/*
IMGUI - ON TEST PHASE AT THE MOMENT
*/ 


TYWRENDERER_API bool		ImGui_ImplGlfwVulkan_Init(VulkanRendererInitializer *m_pWRenderer, uint32_t *framebufferwidth, uint32_t *framebufferheight, const std::string& strVertexShaderPath, const std::string& strFragmentShaderPath, bool install_callbacks);
TYWRENDERER_API void        ImGui_ImplGlfwVulkan_Shutdown();
TYWRENDERER_API void        ImGui_ImplGlfwVulkan_NewFrame(double current_time);
TYWRENDERER_API void        ImGui_ImplGlfwVulkan_Render(VkCommandBuffer command_buffer, uint32_t& bufferIndex);
TYWRENDERER_API void		ImGui_Render();


// Use if you want to reset your rendering device without losing ImGui state.
TYWRENDERER_API void        ImGui_ImplGlfwVulkan_InvalidateFontUploadObjects();
TYWRENDERER_API void        ImGui_ImplGlfwVulkan_InvalidateDeviceObjects();
TYWRENDERER_API bool        ImGui_ImplGlfwVulkan_CreateFontsTexture(VkCommandBuffer command_buffer);
TYWRENDERER_API bool        ImGui_ImplGlfwVulkan_CreateDeviceObjects(const std::string& strVertexShaderPath, const std::string& strFragmentShaderPath);

//Events. Yes horrible for now. But will work....
TYWRENDERER_API	void		ImGui_ImplGlfwVulkan_SetMousePos(const uint32_t& mouseposx, const uint32_t& mouseposy);
TYWRENDERER_API void		ImGui_ImplGlfwVulkan_MousePressed(bool bPressed);
TYWRENDERER_API void		ImGui_ImplGlfwVulkan_MouseOnWinodws(bool bOnWindows);
//





/*
============================================================
TR_TRISURF
============================================================
*/

extern srfTriangles_t *	R_AllocStaticTriSurf();
extern void				R_AllocStaticTriSurfVerts(srfTriangles_t *tri, int numVerts);
extern void				R_AllocStaticTriSurfIndexes(srfTriangles_t *tri, int numIndexes);

extern void				R_FreeStaticTriSurfSilIndexes(srfTriangles_t *tri);
extern void				R_FreeStaticTriSurf(srfTriangles_t *tri);
extern void				R_FreeStaticTriSurfVerts(srfTriangles_t *tri);
extern void				R_CreateStaticBuffersForTri(srfTriangles_t & tri);
//=============================================