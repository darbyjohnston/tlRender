cmake_minimum_required(VERSION 3.18)

project(tlRender LANGUAGES CXX)

# Build options
set(TLR_ENABLE_PYTHON FALSE CACHE BOOL "Enable Python support")
set(TLR_BUILD_TESTS TRUE CACHE BOOL "Build tests")

# Configure
list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
if(TLR_ENABLE_PYTHON)
    add_definitions(-DTLR_ENABLE_PYTHON)
endif()
if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    add_definitions(-DTLR_ASSERT)
endif()
if(TLR_BUILD_TESTS)
    enable_testing()
endif()
include_directories(${PROJECT_SOURCE_DIR}/lib ${PROJECT_SOURCE_DIR}/etc)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Dependencies
find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
find_package(FSeq REQUIRED)
#find_package(PNG REQUIRED)
#find_package(JPEG REQUIRED)
#find_package(TIFF REQUIRED)
find_package(FreeType REQUIRED)
find_package(FFmpeg REQUIRED)
find_package(OTIO REQUIRED)
find_package(glad REQUIRED)
find_package(GLFW REQUIRED)
#find_package(RtAudio REQUIRED)
if(TLR_ENABLE_PYTHON)
    find_package(Python3 REQUIRED COMPONENTS Development)
endif()

# Sub-directories
add_subdirectory(lib)
add_subdirectory(bin)
if(TLR_BUILD_TESTS)
    add_subdirectory(tests)
endif()
