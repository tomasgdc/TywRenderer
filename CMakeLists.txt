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


IF(WIN32)
  ADD_DEFINITIONS(-DUNICODE -D_UNICODE)
ENDIF()

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules")
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Lib)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/Bin)
INCLUDE(cotire)

# includes cmake/FindSDL2.cmake
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMakeModules)

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

find_package(ASSIMP REQUIRED)
include_directories(${ASSIMP_INCLUDE_DIR})

find_package(FREETYPE REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIR})

IF(WIN32)
	find_library(VULKAN_LIB NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/App)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ENDIF(WIN32)

message(STATUS "Vulkan lib path:   " 	${VULKAN_LIB})
message(STATUS "SDL2 lib path: 	   "   	${SDL2_LIBRARY})
message(STATUS "Assimp lib path:   " 	${ASSIMP_LIBRARY})
message(STATUS "Freetype lib path: " 	${FREETYPE_LIBRARY})

ADD_SUBDIRECTORY(Renderer)
ADD_SUBDIRECTORY(Projects)

MESSAGE("Generated with config types: ${CMAKE_CONFIGURATION_TYPES}")
if(CMAKE_CONFIGURATION_TYPES)
    MESSAGE("Multi-configuration generator")
else()
    MESSAGE("Single-configuration generator")
endif()