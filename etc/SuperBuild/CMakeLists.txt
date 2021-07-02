cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0091 NEW)

project(tlRender-SuperBuild)

# Build options
set(TLR_ENABLE_MMAP TRUE CACHE BOOL "Enable memory-mapped file I/O")
set(TLR_ENABLE_PYTHON FALSE CACHE BOOL "Enable Python support (for OTIO Python adapters)")
if(WIN32)
    message(WARNING "See the README for how to build FFmpeg on Windows")
else()
    set(TLR_BUILD_FFmpeg TRUE CACHE BOOL "Build FFmpeg support (Linux and macOS only)")
endif()
set(TLR_BUILD_JPEG TRUE CACHE BOOL "Build JPEG support")
set(TLR_BUILD_PNG TRUE CACHE BOOL "Build PNG support")
set(TLR_BUILD_OpenEXR TRUE CACHE BOOL "Build OpenEXR support")
set(TLR_BUILD_TIFF TRUE CACHE BOOL "Build TIFF support")
set(TLR_BUILD_GL TRUE CACHE BOOL "Build OpenGL library (tlRenderGL)")
set(TLR_BUILD_QT FALSE CACHE BOOL "Build Qt library (tlRenderQt)")
set(TLR_BUILD_EXAMPLES TRUE CACHE BOOL "Build examples")
set(TLR_BUILD_TESTS TRUE CACHE BOOL "Build tests")

# Configure
list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../../cmake/Modules
    ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)

# Common arguments for building external projects.
set(TLR_EXTERNAL_ARGS
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
    -DCMAKE_FIND_FRAMEWORK=${CMAKE_FIND_FRAMEWORK}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS})

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
include(BuildFSeq)
include(BuildZLIB)
include(BuildIlmBase)
include(BuildOCIO)
include(BuildOTIO)
#include(BuildRtAudio)
if(TLR_BUILD_FFmpeg)
    include(BuildNASM)
    include(BuildFFmpeg)
endif()
if(TLR_BUILD_JPEG)
    include(BuildJPEG)
endif()
if(TLR_BUILD_OpenEXR)
    include(BuildOpenEXR)
endif()
if(TLR_BUILD_PNG)
    include(BuildPNG)
endif()
if(TLR_BUILD_TIFF)
    include(BuildTIFF)
endif()
if(TLR_BUILD_GL)
    include(BuildFreeType)
    include(Buildglad)
    if(TLR_BUILD_EXAMPLES)
        include(BuildGLFW)
    endif()
endif()
include(BuildtlRender)
