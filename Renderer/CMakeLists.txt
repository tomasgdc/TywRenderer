CMAKE_MINIMUM_REQUIRED(VERSION 3.4)

PROJECT(TywRenderer)

SET(HEADERS_SCENE_MANAGER
	"SceneManager/ISceneNode.h"
	"SceneManager/SceneNode.h"
	"SceneManager/CameraNode.h"
	"SceneManager/RootNode.h"
	"SceneManager/SceneNodeProperties.h"
	"SceneManager/RenderPass.hpp"
)
SET(SOURCES_SCENE_MANAGER
	"SceneManager/SceneNode.cpp"
	"SceneManager/CameraNode.cpp"
	"SceneManager/RootNode.cpp"
	"SceneManager/SceneNodeProperties.cpp"
	"SceneManager/SceneManager.cpp"
)
SOURCE_GROUP("SceneManager\\Header Files" FILES ${HEADERS_SCENE_MANAGER})
SOURCE_GROUP("SceneManager\\Source Files" FILES ${SOURCES_SCENE_MANAGER})


SET(HEADERS_THIRDPARTY_IMGUI
	"ThirdParty/ImGui/imconfig.h"
	"ThirdParty/ImGui/imgui.h"
	"ThirdParty/ImGui/imgui_internal.h"
	"ThirdParty/ImGui/stb_rect_pack.h"
	"ThirdParty/ImGui/stb_textedit.h"
	"ThirdParty/ImGui/stb_truetype.h"

)
SET(SOURCES_THIRDPARTY_IMGUI
	"ThirdParty/ImGui/imgui.cpp"
	"ThirdParty/ImGui/imgui_demo.cpp"
	"ThirdParty/ImGui/imgui_draw.cpp"
)
SOURCE_GROUP("ThirdParty\\ImGui\\Header Files" FILES ${HEADERS_THIRDPARTY_IMGUI})
SOURCE_GROUP("ThirdParty\\ImGui\\Source Files" FILES ${SOURCES_THIRDPARTY_IMGUI})


SET(HEADERS_MESHLOADER
	"MeshLoader/Model.h"
	"MeshLoader/Model_local.h"
	"MeshLoader/Model_obj.h"
	"MeshLoader/ModelManager.h"
	"MeshLoader/ImageManager.h"
	"MeshLoader/Material.h"
)
SET(SOURCES_MESHLOADER
	"MeshLoader/Model.cpp"
	"MeshLoader/Model_obj.cpp"
	"MeshLoader/Model_md5.cpp"
	"MeshLoader/ModelManager.cpp"
	"MeshLoader/ImageManager.cpp"
	"MeshLoader/Material.cpp"
	"MeshLoader/Model_sprite.cpp"
	"MeshLoader/Model_assimp.cpp"
)
SOURCE_GROUP("MeshLoader\\Header Files" FILES ${HEADERS_MESHLOADER})
SOURCE_GROUP("MeshLoader\\Source Files" FILES ${SOURCES_MESHLOADER})


SET(HEADERS_GEOMETRY
	"Geometry/JointTransform.h"
	"Geometry/VertData.h"
	"Geometry/TangentAndBinormalCalculator.hpp"
)
SET(SOURCES_GEOMETRY
	"Geometry/JointTransform.cpp"
)
SOURCE_GROUP("Geometry\\Header Files" FILES ${HEADERS_GEOMETRY})
SOURCE_GROUP("Geometry\\Source Files" FILES ${SOURCES_GEOMETRY})



SET(HEADERS_VULKAN
	"Vulkan/VkBufferObject.h"
	"Vulkan/VulkanTextureLoader.h"
	"Vulkan/VulkanDebug.h"
	"Vulkan/VulkanAndroid.h"
	"Vulkan/VulkanSwapChain.h"
	"Vulkan/VulkanTools.h"
)
SET(SOURCES_VULKAN
	"Vulkan/VkBufferObject.cpp"
	"Vulkan/VulkanTextureLoader.cpp"
	"Vulkan/VulkanDebug.cpp"
	"Vulkan/VulkanAndroid.cpp"
	"Vulkan/VulkanTools.cpp"
	"Vulkan/VulkanSwapChain.cpp"
)
SOURCE_GROUP("Vulkan\\Header Files" FILES ${HEADERS_VULKAN})
SOURCE_GROUP("Vulkan\\Source Files" FILES ${SOURCES_VULKAN})

SET(HEADERS
	"VulkanRendererInitializer.h"
	"VKRenderer.h"
	"IRendererInitializer.h"
	"IRenderer.h"
	"VertexCache.h"
)
SET(SOURCES
	"VulkanRendererInitializer.cpp"
	"VKRendererGUI.cpp"
	"VKRenderer.cpp"
	"IRendererInitializer.cpp"
	"VertexCache.cpp"
	tri_trisurf.cpp
)
SOURCE_GROUP("Header Files" FILES ${HEADERS})
SOURCE_GROUP("Source Files" FILES ${SOURCES})

	
ADD_LIBRARY(${PROJECT_NAME} SHARED
	${HEADERS_SCENE_MANAGER}
	${SOURCES_SCENE_MANAGER}
	${HEADERS_THIRDPARTY_IMGUI}	
	${SOURCES_THIRDPARTY_IMGUI}
	${HEADERS_MESHLOADER}
	${SOURCES_MESHLOADER}
	${HEADERS_GEOMETRY}
	${SOURCES_GEOMETRY}
	${HEADERS_VULKAN}
	${SOURCES_VULKAN}
	${HEADERS}
	${SOURCES}
)

	
TARGET_LINK_LIBRARIES(${PROJECT_NAME} PUBLIC
	 ${VULKAN_LIB}
	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ThirdParty/assimp-vc140-mt.lib"
	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ThirdParty/libpng16.lib"
	)



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	)

	
set_target_properties(${PROJECT_NAME} PROPERTIES 
	COTIRE_CXX_PREFIX_HEADER_INIT "${CMAKE_SOURCE_DIR}/RendererPch/stdafx.h"
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)
cotire(${PROJECT_NAME})