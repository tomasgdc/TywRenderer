CMAKE_MINIMUM_REQUIRED(VERSION 3.4)


PROJECT(ShadowMapping)


SET(HEADERS
	"Main.h"
)
SET(SOURCES
	"Main.cpp"
)
SOURCE_GROUP("Header Files" FILES ${HEADERS})
SOURCE_GROUP("Source Files" FILES ${SOURCES})


SET(RESOURCES
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/shadowmapping.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/shadowmapping.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/offscreen.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/offscreen.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/quad.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/quad.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/plane.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/ShadowMapping/plane.vert"
)
SOURCE_GROUP("Shader Files" FILES ${RESOURCES})

	
ADD_EXECUTABLE(${PROJECT_NAME}
	${HEADERS}
	${SOURCES}
	${RESOURCES}
)

	
TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
	${VULKAN_LIB}
	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ThirdParty/assimp-vc140-mt.lib"
	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/ThirdParty/freetype265MT.lib"
	debug 		"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Debug/TywRenderer_d.lib"
	optimized 	"${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/Release/TywRenderer.lib"
	)



TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	)

set_target_properties(${PROJECT_NAME} PROPERTIES 
	DEBUG_POSTFIX ${CMAKE_DEBUG_POSTFIX}
	)