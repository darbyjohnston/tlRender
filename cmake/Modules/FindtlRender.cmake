# Find the tlRender library.
#
# This module defines the following variables:
#
# * tlRender_FOUND
# * tlRender_VERSION
# * tlRender_INCLUDE_DIRS
# * tlRender_LIBRARIES
#
# This module defines the following imported targets:
#
# * tlRender::tlCore
# * tlRender::tlIO
# * tlRender::tlTimeline
# * tlRender::tlDevice
# * tlRender::tlGL
# * tlRender::glad
#
# This module defines the following interfaces:
#
# * tlRender

set(tlRender_VERSION 0.0.1)

find_package(GLM REQUIRED)
find_package(Imath REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(FreeType REQUIRED)
find_package(OpenColorIO REQUIRED)
find_package(OTIO REQUIRED)
find_package(libsamplerate REQUIRED)
find_package(RtAudio REQUIRED)
find_package(JPEG)
find_package(TIFF)
find_package(PNG)
find_package(OpenEXR)
find_package(FFmpeg)

find_path(tlRender_INCLUDE_DIR NAMES tlCore/Util.h PATH_SUFFIXES tlRender)
set(tlRender_INCLUDE_DIRS
    ${tlRender_INCLUDE_DIR}
    ${GLM_INCLUDE_DIRS}
    ${Imath_INCLUDE_DIRS}
    ${nlohmann_json_INCLUDE_DIRS}
    ${FreeType_INCLUDE_DIRS}
    ${OTIO_INCLUDE_DIRS}
    ${libsamplerate_INCLUDE_DIRS}
    ${RtAudio_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRS}
    ${TIFF_INCLUDE_DIRS}
    ${PNG_INCLUDE_DIRS}
    ${OpenEXR_INCLUDE_DIRS}
    ${FFmpeg_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_glad_LIBRARY NAMES glad)
else()
    find_library(tlRender_tlCore_LIBRARY NAMES tlCore)
    find_library(tlRender_tlIO_LIBRARY NAMES tlIO)
    find_library(tlRender_tlTimeline_LIBRARY NAMES tlTimeline)
    find_library(tlRender_tlDevice_LIBRARY NAMES tlDevice)
    find_library(tlRender_tlGL_LIBRARY NAMES tlGL)
    find_library(tlRender_glad_LIBRARY NAMES glad)
endif()

set(tlRender_LIBRARIES
    ${tlRender_tlCore_LIBRARY}
    ${tlRender_tlIO_LIBRARY}
    ${tlRender_tlTimeline_LIBRARY}
    ${tlRender_tlDevice_LIBRARY}
    ${tlRender_tlGL_LIBRARY}
    ${tlRender_glad_LIBRARY}
    ${GLM_LIBRARIES}
    ${Imath_LIBRARIES}
    ${nlohmann_json_LIBRARIES}
    ${FreeType_LIBRARIES}
    ${OTIO_LIBRARIES}
    ${libsamplerate_LIBRARIES}
    ${RtAudio_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${TIFF_LIBRARIES}
    ${PNG_LIBRARIES}
    ${OpenEXR_LIBRARIES}
    ${FFmpeg_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    tlRender
    REQUIRED_VARS
        tlRender_INCLUDE_DIR
        tlRender_tlCore_LIBRARY
        tlRender_tlIO_LIBRARY
        tlRender_tlTimeline_LIBRARY
        tlRender_tlDevice_LIBRARY
        tlRender_tlGL_LIBRARY
        tlRender_glad_LIBRARY)
mark_as_advanced(
    tlRender_INCLUDE_DIR
    tlRender_tlCore_LIBRARY
    tlRender_tlIO_LIBRARY
    tlRender_tlTimeline_LIBRARY
    tlRender_tlDevice_LIBRARY
    tlRender_tlGL_LIBRARY
    tlRender_glad_LIBRARY)

set(tlRender_COMPILE_DEFINITIONS tlRender_FOUND)

if(tlRender_FOUND AND NOT TARGET tlRender::tlCore)
    add_library(tlRender::tlCore UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlCore PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlCore_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OTIO;OpenColorIO::OpenColorIO;Imath::Imath;RtAudio;libsamplerate;FreeType;nlohmann_json::nlohmann_json;GLM")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlIO)
    add_library(tlRender::tlIO UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlIO PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlIO_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "JPEG;TIFF;PNG;OpenEXR::OpenEXR;FFmpeg")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlTimeline)
    add_library(tlRender::tlTimeline UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlTimeline PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlTimeline_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlDevice)
    add_library(tlRender::tlDevice UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlDevice PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlDevice_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::tlGL)
    add_library(tlRender::tlGL UNKNOWN IMPORTED)
    set_target_properties(tlRender::tlGL PROPERTIES
        IMPORTED_LOCATION "${tlRender_tlGL_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender::glad)
    add_library(tlRender::glad UNKNOWN IMPORTED)
    set_target_properties(tlRender::glad PROPERTIES
        IMPORTED_LOCATION "${tlRender_glad_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${tlRender_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${tlRender_INCLUDE_DIR}")
endif()
if(tlRender_FOUND AND NOT TARGET tlRender)
    add_library(tlRender INTERFACE)
    target_link_libraries(tlRender INTERFACE tlRender::tlCore)
    target_link_libraries(tlRender INTERFACE tlRender::tlIO)
    target_link_libraries(tlRender INTERFACE tlRender::tlTimeline)
    target_link_libraries(tlRender INTERFACE tlRender::tlDevice)
    target_link_libraries(tlRender INTERFACE tlRender::tlGL)
    target_link_libraries(tlRender INTERFACE tlRender::glad)
endif()
