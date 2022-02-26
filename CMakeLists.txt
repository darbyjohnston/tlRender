cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0091 NEW)

set(TLRENDER_VERSION_MAJOR "0")
set(TLRENDER_VERSION_MINOR "1")
set(TLRENDER_VERSION_PATCH "0")
set(TLRENDER_VERSION ${TLRENDER_VERSION_MAJOR}.${TLRENDER_VERSION_MINOR}.${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION_MAJOR=${TLRENDER_VERSION_MAJOR})
add_definitions(-DTLRENDER_VERSION_MINOR=${TLRENDER_VERSION_MINOR})
add_definitions(-DTLRENDER_VERSION_PATCH=${TLRENDER_VERSION_PATCH})
add_definitions(-DTLRENDER_VERSION="${TLRENDER_VERSION}")

project(
	tlRender
    VERSION ${TLRENDER_VERSION}
    DESCRIPTION "tlRender is an open source library for building playback and review applications for visual effects, film, and animation"
    HOMEPAGE_URL "https://github.com/darbyjohnston/tlRender"
	LANGUAGES CXX C)

#-------------------------------------------------------------------------------
# Build options

set(TLRENDER_ENABLE_MMAP TRUE CACHE BOOL "Enable memory-mapped file I/O")
set(TLRENDER_ENABLE_COVERAGE FALSE CACHE BOOL "Enable code coverage")
set(TLRENDER_ENABLE_PYTHON FALSE CACHE BOOL "Enable Python support (for OTIO Python adapters)")
set(TLRENDER_BUILD_GL TRUE CACHE BOOL "Build OpenGL support")
set(TLRENDER_BUILD_QT6 FALSE CACHE BOOL "Build Qt6 support")
set(TLRENDER_BUILD_QT5 FALSE CACHE BOOL "Build Qt5 support")
set(TLRENDER_BUILD_PROGRAMS TRUE CACHE BOOL "Build programs")
set(TLRENDER_BUILD_EXAMPLES TRUE CACHE BOOL "Build examples")
set(TLRENDER_BUILD_TESTS TRUE CACHE BOOL "Build tests")

#-------------------------------------------------------------------------------
# Internal configuration

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

if(TLRENDER_ENABLE_MMAP)
    add_definitions(-DTLRENDER_ENABLE_MMAP)
endif()
if(TLRENDER_ENABLE_PYTHON)
    add_definitions(-DTLRENDER_ENABLE_PYTHON)
endif()
if(TLRENDER_BUILD_GL)
    add_definitions(-DTLRENDER_BUILD_GL)
endif()
if(TLRENDER_BUILD_QT6)
    add_definitions(-DTLRENDER_BUILD_QT6)
elseif(TLRENDER_BUILD_QT5)
    add_definitions(-DTLRENDER_BUILD_QT5)
endif()
if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    add_definitions(-DTLRENDER_ASSERT)
endif()
if(TLRENDER_BUILD_TESTS)
    set(CTEST_OUTPUT_ON_FAILURE ON)
    enable_testing()
endif()

include_directories(
	${PROJECT_SOURCE_DIR}/deps
	${PROJECT_SOURCE_DIR}/lib
	${PROJECT_SOURCE_DIR}/etc
	${PROJECT_SOURCE_DIR}/tests)

if(TLRENDER_ENABLE_COVERAGE)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_CXX_FLAGS} --coverage")
endif()

set(CMAKE_FIND_FRAMEWORK LAST)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------
# Dependencies

find_package(Threads REQUIRED)
find_package(FSeq REQUIRED)
find_package(GLM REQUIRED)
find_package(Imath REQUIRED)
find_package(FreeType REQUIRED)
find_package(OCIO REQUIRED)
find_package(OTIO REQUIRED)
find_package(libsamplerate REQUIRED)
find_package(RtAudio REQUIRED)
find_package(JPEG)
find_package(TIFF)
find_package(PNG)
find_package(OpenEXR)
find_package(FFmpeg)
if(TLRENDER_ENABLE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()
if(TLRENDER_BUILD_GL)
    find_package(OpenGL REQUIRED)
    if(TLRENDER_BUILD_EXAMPLES)
        find_package(GLFW REQUIRED)
    endif()
endif()
if(TLRENDER_BUILD_QT6)
    find_package(OpenGL REQUIRED)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 COMPONENTS Quick Widgets OpenGLWidgets Svg HINTS "$ENV{QTDIR}")
elseif(TLRENDER_BUILD_QT5)
    find_package(OpenGL REQUIRED)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt5 REQUIRED COMPONENTS Quick Widgets Svg HINTS "$ENV{QTDIR}")
endif()

#-------------------------------------------------------------------------------
# Sub-directories

if(TLRENDER_BUILD_GL)
	add_subdirectory(deps/tlGlad)
endif()
add_subdirectory(lib)
if(TLRENDER_BUILD_PROGRAMS)
    add_subdirectory(bin)
endif()
if(TLRENDER_BUILD_TESTS)
    add_subdirectory(tests)
endif()
if(TLRENDER_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
