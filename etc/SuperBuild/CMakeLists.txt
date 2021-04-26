cmake_minimum_required(VERSION 3.18)

project(tlRender-SuperBuild)

# Build options
set(TLR_ENABLE_PYTHON FALSE CACHE BOOL "Enable Python support")
set(TLR_BUILD_TESTS TRUE CACHE BOOL "Build tests")

# Configure
list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../../cmake/Modules
    ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_CXX_STANDARD 14)

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
include(BuildFSeq)
include(BuildZLIB)
#include(BuildPNG)
#include(BuildJPEG)
#include(BuildTIFF)
include(BuildFreeType)
if(NOT WIN32)
    include(BuildNASM)
    include(BuildFFmpeg)
endif()
include(BuildOTIO)
include(Buildglad)
include(BuildGLFW)
#include(BuildRtAudio)
include(BuildtlRender)
