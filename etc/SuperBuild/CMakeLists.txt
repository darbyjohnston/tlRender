cmake_minimum_required(VERSION 3.31)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

project(tlRender-SuperBuild)

#-------------------------------------------------------------------------------
# Build options

set(TLRENDER_NET FALSE CACHE BOOL "Enable network support")
set(TLRENDER_OCIO TRUE CACHE BOOL "Enable support for OpenColorIO")
set(TLRENDER_JPEG TRUE CACHE BOOL "Enable support for JPEG")
set(TLRENDER_TIFF TRUE CACHE BOOL "Enable support for TIFF")
set(TLRENDER_STB TRUE CACHE BOOL "Enable support for STB I/O (TGA, BMP, PSD)")
set(TLRENDER_PNG TRUE CACHE BOOL "Enable support for PNG")
set(TLRENDER_EXR TRUE CACHE BOOL "Enable support for OpenEXR")
set(TLRENDER_AV1_DEFAULT OFF)
if(WIN32)
    set(TLRENDER_AV1_DEFAULT OFF)
endif()
set(TLRENDER_AV1 ${TLRENDER_AV1_DEFAULT} CACHE BOOL "Enable support for AV1")
set(TLRENDER_FFMPEG TRUE CACHE BOOL "Enable support for FFmpeg (see the README for building FFmpeg on Windows)")
set(TLRENDER_FFMPEG_MINIMAL FALSE CACHE BOOL "Enable a minimal set of FFmpeg codecs")
set(TLRENDER_USD FALSE CACHE BOOL "Enable support for USD")
set(TLRENDER_USD_PYTHON python3 CACHE STRING "Python executable for building USD")
set(TLRENDER_BMD FALSE CACHE BOOL "Enable support for Blackmagic Design devices")
set(TLRENDER_BMD_SDK "" CACHE PATH "Full path to the Blackmagic Design SDK")
set(TLRENDER_QT6 FALSE CACHE BOOL "Enable support for Qt6")
set(TLRENDER_QT5 FALSE CACHE BOOL "Enable support for Qt5")
set(TLRENDER_PROGRAMS TRUE CACHE BOOL "Build programs")
set(TLRENDER_EXAMPLES TRUE CACHE BOOL "Build examples")
set(TLRENDER_TESTS TRUE CACHE BOOL "Build tests")
if(APPLE)
    set(TLRENDER_IGNORE_PREFIX_PATH_DEFAULT /opt/homebrew)
endif()
set(TLRENDER_IGNORE_PREFIX_PATH ${TLRENDER_IGNORE_PREFIX_PATH_DEFAULT} CACHE STRING "Ignore the given prefix path")
set(TLRENDER_GCOV FALSE CACHE BOOL "Enable gcov code coverage")
set(TLRENDER_GPROF FALSE CACHE BOOL "Enable gprof code profiling")
set(feather_tk_API "GL_4_1" CACHE STRING "Graphics API (GL_4_1, GL_4_1_Debug, GLES_2)")

#-------------------------------------------------------------------------------
# Internal configuration

# \bug Add the OpenColorIO cmake modules.
list(PREPEND CMAKE_MODULE_PATH
    ${PROJECT_SOURCE_DIR}/../../cmake/Modules
    ${PROJECT_SOURCE_DIR}/cmake/Modules
    ${CMAKE_INSTALL_PREFIX}/share/OpenColorIO/cmake/modules)

if(NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 17)
endif()

if(NOT BUILD_SHARED_LIBS)
    set(BUILD_SHARED_LIBS OFF)
endif()

if(TLRENDER_IGNORE_PREFIX_PATH)
    set(CMAKE_IGNORE_PREFIX_PATH ${TLRENDER_IGNORE_PREFIX_PATH})
endif()

if(WIN32)
    set(CMAKE_MSVC_RUNTIME_LIBRARY MultiThreaded$<$<CONFIG:Debug>:Debug>DLL)
endif()

if(APPLE)
    set(CMAKE_FIND_FRAMEWORK LAST)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

# https://stackoverflow.com/questions/45414507/pass-a-list-of-prefix-paths-to-externalproject-add-in-cmake-args
string(REPLACE ";" "|" CMAKE_MODULE_PATH_TMP "${CMAKE_MODULE_PATH}")
string(REPLACE ";" "|" CMAKE_PREFIX_PATH_TMP "${CMAKE_PREFIX_PATH}")

# Common arguments for building external projects.
set(TLRENDER_EXTERNAL_ARGS
    -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH_TMP}
    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
    -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH_TMP}
    -DCMAKE_IGNORE_PREFIX_PATH=${CMAKE_IGNORE_PREFIX_PATH}
    -DCMAKE_POLICY_DEFAULT_CMP0091:STRING=NEW
    -DCMAKE_POLICY_DEFAULT_CMP0114:STRING=NEW
    -DCMAKE_FIND_FRAMEWORK=${CMAKE_FIND_FRAMEWORK}
    -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD}
    -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
    -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
    -DCMAKE_MSVC_RUNTIME_LIBRARY=${CMAKE_MSVC_RUNTIME_LIBRARY}
    -DCMAKE_VERBOSE_MAKEFILE=${CMAKE_VERBOSE_MAKEFILE}
    -DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}
    -DCMAKE_OSX_ARCHITECTURES=${CMAKE_OSX_ARCHITECTURES}
    -DBUILD_SHARED_LIBS=${BUILD_SHARED_LIBS})
string(REPLACE ";" " " TLRENDER_EXTERNAL_ARGS_TMP "${TLRENDER_EXTERNAL_ARGS}")
message("TLRENDER_EXTERNAL_ARGS: " ${TLRENDER_EXTERNAL_ARGS_TMP})

#-------------------------------------------------------------------------------
# Dependencies

find_package(Threads REQUIRED)
include(BuildZLIB)
if(TLRENDER_PNG)
    include(BuildPNG)
endif()
include(Buildfeather-tk)
include(Buildminizip-ng)
include(BuildImath)
include(BuildOTIO)
if((TLRENDER_NET OR TLRENDER_AV1 OR TLRENDER_JPEG OR TLRENDER_FFMPEG) AND NOT WIN32 AND NOT BSD)
    include(BuildNASM)
    set(TLRENDER_EXTERNAL_DEPS_NASM ON)
endif()
if(TLRENDER_NET)
    include(BuildOpenSSL)
    include(BuildLibssh2)
    include(BuildCURL)
endif()
if(TLRENDER_OCIO)
    include(Buildyaml-cpp)
    include(Buildexpat)
    include(Buildpystring)
    include(BuildOpenColorIO)
endif()
if(TLRENDER_JPEG)
    include(Buildlibjpeg-turbo)
endif()
if(TLRENDER_TIFF)
    include(BuildTIFF)
endif()
if(TLRENDER_EXR)
    include(BuildOpenEXR)
endif()
if(TLRENDER_AV1)
    include(Buildsvt-av1)
endif()
if(TLRENDER_FFMPEG)
    include(BuildFFmpeg)
endif()
if(TLRENDER_USD)
    include(BuildUSD)
endif()
foreach(EXTERNAL_DEP ${TLRENDER_EXTERNAL_DEPS})
    include(Build${EXTERNAL_DEP})
endforeach()

if (TLRENDER_TESTS)
    include(Buildtests)
endif()
