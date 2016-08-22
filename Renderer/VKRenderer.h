//Copyright 2015-2016 Tomas Mikalauskas. All rights reserved.
#pragma once


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

	void VShutdown() override;

	bool VInitRenderer(uint32_t height, uint32_t widht, bool isFullscreen, LRESULT(CALLBACK MainWindowProc)(HWND, UINT, WPARAM, LPARAM)) override;

	void VSetBackgroundColor(BYTE bgA, BYTE bgR, BYTE bgG, BYTE bgB)
	{
		m_backgroundColor[0] = float(bgA) / 255.0f;
		m_backgroundColor[1] = float(bgR) / 255.0f;
		m_backgroundColor[2] = float(bgG) / 255.0f;
		m_backgroundColor[3] = float(bgB) / 255.0f;
	}


	void VWindowResize(uint32_t iHeight, uint32_t iWidth) override;

	bool VPreRender() override;
	bool VPostRender() override;
	HRESULT VOnRestore() override;


	std::shared_ptr<IRenderState> VPrepareAlphaPass() override;
	std::shared_ptr<IRenderState> VPrepareSkyBoxPass() override;


	//Creates files for logging
	void VSetLogPath();

	//Clears screens and swaps buffers
	//void SwapCommandsFinnishRendering(uint64_t* gpuMicroSec);

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
	void PrepareSemaphore();


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
	virtual void VLoadTexture(std::string fileName, VkFormat format, bool forceLinearTiling);

	VkCommandBuffer GetCommandBuffer(bool begin);

	void FlushCommandBuffer(VkCommandBuffer commandBuffer);

	//overridable
	virtual void SetupDescriptorSet();

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
	VkFont								*m_FontRender;
	ImageManager						*m_pImageManager;

protected:
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
		VkSemaphore presentComplete;
		VkSemaphore renderComplete;
	} Semaphores;


	// List of shader modules created (stored for cleanup)
	std::vector<VkShaderModule> m_ShaderModules;


	VkClearColorValue defaultClearColor = { { 0.5f, 0.5f, 0.5f, 1.0f } };



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


	// Defines a frame rate independent timer value clamped from -1.0...1.0
	// For use in animations, rotations, etc.
	float timer = 0.0f;
	// Multiplier for speeding up (or slowing down) the global timer
	float timerSpeed = 0.25f;
	bool paused = false;


	uint32_t m_WindowWidth;
	uint32_t m_WindowHeight;
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
TESTING
*/
TYWRENDERER_API bool		ImGui_ImplGlfwGL3_Init(IRendererInitializer *m_pWRenderer, bool install_callbacks);
TYWRENDERER_API void        ImGui_ImplGlfwGL3_Shutdown();
TYWRENDERER_API void		ImGui_ImplGlfwGL3_NewFrame(double dCurrentTime,  struct Win32vars & winVars);

// Use if you want to reset your rendering device without losing ImGui state.
TYWRENDERER_API void        ImGui_ImplGlfwGL3_InvalidateDeviceObjects();
TYWRENDERER_API bool        ImGui_ImplGlfwGL3_CreateDeviceObjects();
TYWRENDERER_API bool		ImGui_ImplGlfwGL3_CreateFontsTexture();
TYWRENDERER_API bool		ImguiRender();
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