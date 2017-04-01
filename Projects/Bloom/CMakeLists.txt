CMAKE_MINIMUM_REQUIRED(VERSION 3.4)


PROJECT(Bloom)


SET(HEADERS
	"Main.h"
)
SET(SOURCES
	"Main.cpp"
)
SOURCE_GROUP("Header Files" FILES ${HEADERS})
SOURCE_GROUP("Source Files" FILES ${SOURCES})


SET(RESOURCES
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DefferedModel.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DefferedModel.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DefferedMRT.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DefferedMRT.vert"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DebugQuad.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/Bloom/DebugQuad.vert"
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
	${ASSIMP_LIBRARY}
	${FREETYPE_LIBRARY}
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