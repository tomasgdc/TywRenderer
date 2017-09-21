#-------------------------------------------------------------------
#		Copyright 2016 Tomas Mikalauskas
# This file is part of the CMake build system for TywRenderer
#     (Object-oriented Graphics Rendering Engine)
#
# The contents of this file are placed in the public domain. Feel
# free to make use of it in any way you like.
#-----------------------------------------------------------------


######################################################################
# TYWLL RENDERER BUILD SYSTEM
# Welcome to the CMake build system for TywRenderer
# This is the main file where we prepare the general build environment
# and provide build configuration options.
######################################################################


CMAKE_MINIMUM_REQUIRED(VERSION 3.4)
PROJECT(TywRenderer)


# Reduce the config types to only Debug and Release
SET(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} --my-debug-flags")
SET(CMAKE_DEBUG_POSTFIX "_d")

#Add External Include Dir that for GLI
INCLUDE_DIRECTORIES(
	"${CMAKE_SOURCE_DIR}/External/glm"
	"${CMAKE_SOURCE_DIR}/External/gli"
	"${CMAKE_SOURCE_DIR}/External/assimp"
	"${CMAKE_SOURCE_DIR}/External/freetype/include"
)

# Set preprocessor defines
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
add_definitions(-std=c++14)
add_definitions(-std=c++0x)
add_definitions(-std=gnu++14)

if (ANDROID)
	set(CMAKE_ANDROID_GUI TRUE)
	set(CMAKE_ANDROID_API 24)
	set(CMAKE_ANDROID_API_MIN 24)
	
	if(DEFINED ANDROID_NDK)
		add_definitions(-DVK_USE_PLATFORM_ANDROID_KHR)
	endif()
endif()


IF(WIN32)
  ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ENDIF()

SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/CMakeModules")

include(cotire)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIR})

set(FREETPYE_PATH ${CMAKE_SOURCE_DIR}/External/freetype)
set(FREETYPE_LIB_SEARCH_PATH "${CMAKE_SOURCE_DIR}/External/freetype/objs/vc2017/x64")

find_package(Freetype_custom QUIET)
include_directories(${FREETYPE_INCLUDE_DIR})

#As we are building Assimp as static library. We need to get zlib. Luckily assimp contains zlib and builds it
set(ZLIB_LIB_SEARCH_PATH ${CMAKE_SOURCE_DIR}/External/assimp/build/contrib/zlib)
find_package(Zlib REQUIRED)

IF(WIN32)
	find_library(VULKAN_LIB NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/App)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ENDIF(WIN32)

message(STATUS "Vulkan lib path:   " 	${VULKAN_LIB})
message(STATUS "SDL2 lib path: 	   "   	${SDL2_LIBRARY})
message(STATUS "Assimp lib path:   " 	${ASSIMP_LIBRARIES})
message(STATUS "Freetype lib path: " 	${FREETYPE_LIBRARIES})
message(STATUS "Freetype include path: " ${FREETYP_INCLUDE_DIR})
message(STATUS "Zlib lib path: "        ${ZLIB_LIBRARIES}) 


ADD_SUBDIRECTORY(Renderer)
ADD_SUBDIRECTORY(Projects)

MESSAGE("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
if(CMAKE_CONFIGURATION_TYPES)
    MESSAGE("Multi-configuration generator")
else()
    MESSAGE("Single-configuration generator")
endif()