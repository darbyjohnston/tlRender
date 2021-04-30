cmake_minimum_required(VERSION 3.18)

project(tlRender-SuperBuild)

# Build options
set(TLR_ENABLE_PYTHON FALSE CACHE BOOL "Enable Python support")
set(TLR_BUILD_JPEG TRUE CACHE BOOL "Build JPEG")
set(TLR_BUILD_OpenEXR TRUE CACHE BOOL "Build OpenEXR")
if(WIN32)
    message(WARNING "See the README for how to build FFmpeg on Windows")
else()
    set(TLR_BUILD_FFmpeg TRUE CACHE BOOL "Build FFmpeg")
endif()
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
include(BuildIlmBase)
include(BuildFreeType)
include(BuildOTIO)
include(Buildglad)
include(BuildGLFW)
#include(BuildRtAudio)
if(TLR_BUILD_JPEG)
    include(BuildJPEG)
endif()
if(TLR_BUILD_OpenEXR)
    include(BuildOpenEXR)
endif()
if(TLR_BUILD_FFmpeg)
    include(BuildNASM)
    include(BuildFFmpeg)
endif()
include(BuildtlRender)
