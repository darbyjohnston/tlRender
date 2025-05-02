cmake_minimum_required(VERSION 3.19)
cmake_policy(SET CMP0091 NEW)
cmake_policy(SET CMP0114 NEW)
if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(TLRENDER_VERSION_MAJOR "0")
set(TLRENDER_VERSION_MINOR "2")
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

set(TLRENDER_NET FALSE CACHE BOOL "Enable network support")
set(TLRENDER_OCIO TRUE CACHE BOOL "Enable support for OpenColorIO")
set(TLRENDER_SDL2 TRUE CACHE BOOL "Enable support for SDL2")
set(TLRENDER_SDL3 FALSE CACHE BOOL "Enable support for SDL3")
set(TLRENDER_JPEG TRUE CACHE BOOL "Enable support for JPEG I/O")
set(TLRENDER_TIFF TRUE CACHE BOOL "Enable support for TIFF I/O")
set(TLRENDER_STB TRUE CACHE BOOL "Enable support for STB I/O (TGA, BMP, PSD)")
set(TLRENDER_PNG TRUE CACHE BOOL "Enable support for PNG I/O")
set(TLRENDER_EXR TRUE CACHE BOOL "Enable support for EXR I/O")
set(TLRENDER_FFMPEG TRUE CACHE BOOL "Enable support for FFmpeg I/O")
set(TLRENDER_USD FALSE CACHE BOOL "Enable support for USD")
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

#-------------------------------------------------------------------------------
# Internal configuration

list(PREPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)

if(TLRENDER_USD)
    add_definitions(-DTLRENDER_USD)
endif()
if(TLRENDER_BMD)
    add_definitions(-DTLRENDER_BMD)
endif()
if(TLRENDER_QT6)
    add_definitions(-DTLRENDER_QT6)
elseif(TLRENDER_QT5)
    add_definitions(-DTLRENDER_QT5)
endif()
if(TLRENDER_IGNORE_PREFIX_PATH)
    set(CMAKE_IGNORE_PREFIX_PATH ${TLRENDER_IGNORE_PREFIX_PATH})
endif()

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    add_definitions(-DTLRENDER_ASSERT)
endif()

if(TLRENDER_TESTS)
    add_definitions(-DTLRENDER_SAMPLE_DATA="${CMAKE_CURRENT_SOURCE_DIR}/etc/SampleData")
    set(CTEST_OUTPUT_ON_FAILURE ON)
    enable_testing()
endif()

include_directories(
    ${PROJECT_SOURCE_DIR}/deps
    ${PROJECT_SOURCE_DIR}/lib
    ${PROJECT_SOURCE_DIR}/etc
    ${PROJECT_SOURCE_DIR}/tests)

if(TLRENDER_GPROF)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -pg")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
endif()
if(TLRENDER_GCOV)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --coverage -fprofile-update=atomic")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --coverage -fprofile-update=atomic")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage -fprofile-update=atomic")
endif()

if(APPLE)
    set(CMAKE_FIND_FRAMEWORK LAST)
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /MP")
endif()

if(WIN32)
elseif(APPLE)
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
else()
    set(CMAKE_SKIP_BUILD_RPATH FALSE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH "../lib")
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

#-------------------------------------------------------------------------------
# Dependencies

# Base dependencies
find_package(Imath REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ZLIB REQUIRED)
find_package(minizip REQUIRED)
find_package(OpenTimelineIO REQUIRED)

# OpenColorIO dependencies
if(TLRENDER_OCIO)
    find_package(OpenColorIO REQUIRED)
    add_definitions(-DTLRENDER_OCIO)
endif()

# SDL dependencies
if(TLRENDER_SDL2)
    find_package(SDL2 REQUIRED)
    add_definitions(-DTLRENDER_SDL2)
endif()
if(TLRENDER_SDL3)
    find_package(SDL3 REQUIRED)
    add_definitions(-DTLRENDER_SDL3)
endif()

# I/O dependencies
if(TLRENDER_JPEG)
    find_package(libjpeg-turbo)
    if(libjpeg-turbo_FOUND)
        add_definitions(-DTLRENDER_JPEG)
    endif()
endif()
if(TLRENDER_TIFF)
    find_package(TIFF)
    if(TIFF_FOUND)
        add_definitions(-DTLRENDER_TIFF)
    endif()
endif()
if(TLRENDER_STB)
    add_definitions(-DTLRENDER_STB)
endif()
if(TLRENDER_PNG)
    find_package(PNG)
    if(PNG_FOUND)
        add_definitions(-DTLRENDER_PNG)
    endif()
endif()
if(TLRENDER_EXR)
    find_package(OpenEXR)
    if(OpenEXR_FOUND)
        add_definitions(-DTLRENDER_EXR)
    endif()
endif()
if(TLRENDER_FFMPEG)
    set(FFmpeg_LIBS)
    set(swresample_LIBS)
    if(WIN32)
        set(FFmpeg_FOUND TRUE)
        set(FFmpeg_LIBS
            ${CMAKE_INSTALL_PREFIX}/lib/avdevice.lib
            ${CMAKE_INSTALL_PREFIX}/lib/avformat.lib
            ${CMAKE_INSTALL_PREFIX}/lib/avcodec.lib
            ${CMAKE_INSTALL_PREFIX}/lib/swresample.lib
            ${CMAKE_INSTALL_PREFIX}/lib/swscale.lib
            ${CMAKE_INSTALL_PREFIX}/lib/avutil.lib)
        set(swresample_LIBS
            ${CMAKE_INSTALL_PREFIX}/lib/swresample.lib)
    else()
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(
            swresample REQUIRED IMPORTED_TARGET
            libswresample)
        pkg_check_modules(
            FFmpeg REQUIRED IMPORTED_TARGET
            libavdevice
            libavformat
            libavcodec
            libswresample
            libswscale
            libavutil)
        set(FFmpeg_LIBS PkgConfig::FFmpeg)
        set(swresample_LIBS PkgConfig::swresample)
    endif()
    if(FFmpeg_FOUND)
        add_definitions(-DTLRENDER_FFMPEG)
    endif()
endif()
if(TLRENDER_USD)
    find_package(pxr)
    if(pxr_FOUND)
        add_definitions(-DTLRENDER_USD)
    endif()
endif()

# dtk dependencies
find_package(dtk REQUIRED)

# Qt dependencies
if(TLRENDER_QT6)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt6 REQUIRED COMPONENTS Quick Widgets OpenGLWidgets Svg HINTS "$ENV{QTDIR}")
elseif(TLRENDER_QT5)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)
    find_package(Qt5 REQUIRED COMPONENTS Quick Widgets Svg HINTS "$ENV{QTDIR}")
endif()

#-------------------------------------------------------------------------------
# Sub-directories

add_subdirectory(lib)
if(TLRENDER_PROGRAMS)
    add_subdirectory(bin)
endif()
if(TLRENDER_TESTS)
    add_subdirectory(tests)
endif()
if(TLRENDER_EXAMPLES)
    add_subdirectory(examples)
endif()
add_subdirectory(etc)

include(CMakePackageConfigHelpers)
set(INCLUDE_INSTALL_DIR include/tlRender)
configure_package_config_file(
    tlRenderConfig.cmake.in
    ${CMAKE_CURRENT_BINARY_DIR}/tlRenderConfig.cmake
    INSTALL_DESTINATION lib/cmake/tlRender
    PATH_VARS INCLUDE_INSTALL_DIR)
install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/tlRenderConfig.cmake
    DESTINATION lib/cmake/tlRender)

#-------------------------------------------------------------------------------
# Packaging

if(CMAKE_BUILD_TYPE MATCHES "^Release$")
    include(Package)
    include(CPack)
    include(InstallRequiredSystemLibraries)
endif()
