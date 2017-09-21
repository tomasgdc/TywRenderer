CMAKE_MINIMUM_REQUIRED(VERSION 3.4)


PROJECT(SSAO)


SET(HEADERS
	"Main.h"
)
SET(SOURCES
	"Main.cpp"
)
SOURCE_GROUP("Header Files" FILES ${HEADERS})
SOURCE_GROUP("Source Files" FILES ${SOURCES})


SET(RESOURCES
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DefferedModel.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DefferedMRT.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DefferedMRT.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DebugQuad.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DebugQuad.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/SSAO.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DebugNormals.geom"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DebugNormals.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/DebugNormals.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/Fullscreen.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/SSAO/Blur.frag"
)
SOURCE_GROUP("Shader Files" FILES ${RESOURCES})

	
ADD_EXECUTABLE(${PROJECT_NAME}
	${HEADERS}
	${SOURCES}
	${RESOURCES}
)

	
TARGET_LINK_LIBRARIES(${PROJECT_NAME}  
	${VULKAN_LIB}
	${SDL2_LIBRARY}
	debug ${FREETYPE_LIBRARY_DEBUG}
	debug ${ASSIMP_LIBRARY_DEBUG}
	debug ${ZLIB_LIBRARY_DEBUG}
	debug "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Debug/TywRenderer_d.lib"
	optimized ${FREETYPE_LIBRARY_RELEASE}
	optimized ${ASSIMP_LIBRARY_RELEASE}
	optimized ${ZLIB_LIBRARY_RELEASE}
	optimized 	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Release/TywRenderer.lib"
	)



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	)

set_target_properties(${PROJECT_NAME} PROPERTIES 
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)