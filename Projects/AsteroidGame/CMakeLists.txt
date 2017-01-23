CMAKE_MINIMUM_REQUIRED(VERSION 3.4)


PROJECT(AsteroidGame)


SET(MAIN_FILES
	"Main.cpp"
)
SOURCE_GROUP("Main" FILES ${MAIN_FILES})

SET(EVENT_MANAGER_FILES
	"Event.h"
	"EventManager.h"
	"Event.cpp"
	"EventManager.cpp"
)
SOURCE_GROUP("Event Manager" FILES ${EVENT_MANAGER_FILES})


SET(ACTORS_FILES
	"Actor.h"
	"AsteroidNode.h"
	"BulletNode.h"
	"ShipNode.h"
	"AsteroidNode.cpp"
	"BulletNode.cpp"
	"ShipNode.cpp"
)
SOURCE_GROUP("Actors" FILES ${ACTORS_FILES})


SET(RENDERER_FILES
	"Renderer.h"
	"SceneManager.h"
	"ISceneNode.h"
	"SceneNode.h"
	"SceneNode.cpp"
	"SceneManager.cpp"
	"Renderer.cpp"
)
SOURCE_GROUP("Renderer" FILES ${RENDERER_FILES})


SET(GAME_FILES
	"Game.h"
	"Helpers.h"
	"Game.cpp"
)
SOURCE_GROUP("Game" FILES ${GAME_FILES})


SET(PHYSICS_FILES
	"Physics.h"
)
SOURCE_GROUP("Physics" FILES ${PHYSICS_FILES})


SET(FRAMEWORK_FILES
	"Engine.h"
	"Engine.cpp"
)
SOURCE_GROUP("Framework" FILES ${FRAMEWORK_FILES})



SET(RESOURCES
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/AsteroidGame/triangle.frag"
	"${CMAKE_SOURCE_DIR}/Assets/Shaders/AsteroidGame/triangle.vert"
)
SOURCE_GROUP("Shader Files" FILES ${RESOURCES})

	
ADD_EXECUTABLE(${PROJECT_NAME}
	${MAIN_FILES}
	${RESOURCES}
	${ACTORS_FILES}
	${RENDERER_FILES}
	${PHYSICS_FILES}
	${GAME_FILES}
	${FRAMEWORK_FILES}
	${EVENT_MANAGER_FILES}
)

	
TARGET_LINK_LIBRARIES(${PROJECT_NAME} 
	${VULKAN_LIB}
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