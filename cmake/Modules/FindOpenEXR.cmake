# Find the OpenEXR library.
#
# This module defines the following variables:
#
# * OpenEXR_FOUND
# * OpenEXR_VERSION
# * OpenEXR_INCLUDE_DIRS
# * OpenEXR_LIBRARIES
#
# This module defines the following imported targets:
#
# * OpenEXR::Iex
# * OpenEXR::IlmThread
# * OpenEXR::OpenEXR
# * OpenEXR::OpenEXRUtil
#
# This module defines the following interfaces:
#
# * OpenEXR

set(OpenEXR_VERSION 3.0.5)

find_package(Imath REQUIRED)

find_path(OpenEXR_INCLUDE_DIR NAMES ImfHeader.h PATH_SUFFIXES OpenEXR)
set(OpenEXR_INCLUDE_DIRS
    ${OpenEXR_INCLUDE_DIR}
    ${Imath_INCLUDE_DIRS})

if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(OpenEXR_Iex_LIBRARY NAMES Iex-3_0_d Iex-3_0)
    find_library(OpenEXR_IlmThread_LIBRARY NAMES IlmThread-3_0_d IlmThread-3_0)
    find_library(OpenEXR_OpenEXR_LIBRARY NAMES OpenEXR-3_0_d OpenEXR-3_0)
    find_library(OpenEXR_OpenEXRUtil_LIBRARY NAMES OpenEXRUtil-3_0_d OpenEXRUtil-3_0)
else()
    find_library(OpenEXR_Iex_LIBRARY NAMES Iex-3_0)
    find_library(OpenEXR_IlmThread_LIBRARY NAMES IlmThread-3_0)
    find_library(OpenEXR_OpenEXR_LIBRARY NAMES OpenEXR-3_0)
    find_library(OpenEXR_OpenEXRUtil_LIBRARY NAMES OpenEXRUtil-3_0)
endif()
set(OpenEXR_LIBRARIES
    ${OpenEXR_Iex_LIBRARY}
    ${OpenEXR_IlmThread_LIBRARY}
    ${OpenEXR_OpenEXR_LIBRARY}
    ${OpenEXR_OpenEXRUtil_LIBRARY}
    ${Imath_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OpenEXR
    REQUIRED_VARS
        OpenEXR_INCLUDE_DIR
        OpenEXR_Iex_LIBRARY
        OpenEXR_IlmThread_LIBRARY
        OpenEXR_OpenEXR_LIBRARY
        OpenEXR_OpenEXRUtil_LIBRARY)
mark_as_advanced(
    OpenEXR_INCLUDE_DIR
    OpenEXR_Iex_LIBRARY
    OpenEXR_IlmThread_LIBRARY
    OpenEXR_OpenEXR_LIBRARY
    OpenEXR_OpenEXRUtil_LIBRARY)

set(OpenEXR_COMPILE_DEFINITIONS OpenEXR_FOUND)
if(BUILD_SHARED_LIBS)
    list(APPEND OpenEXR_COMPILE_DEFINITIONS OPENEXR_DLL)
endif()

if(OpenEXR_FOUND AND NOT TARGET OpenEXR::Iex)
    add_library(OpenEXR::Iex UNKNOWN IMPORTED)
    set_target_properties(OpenEXR::Iex PROPERTIES
        IMPORTED_LOCATION "${OpenEXR_Iex_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${OpenEXR_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}")
endif()
if(OpenEXR_FOUND AND NOT TARGET OpenEXR::IlmThread)
    add_library(OpenEXR::IlmThread UNKNOWN IMPORTED)
    set_target_properties(OpenEXR::IlmThread PROPERTIES
        IMPORTED_LOCATION "${OpenEXR_IlmThread_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${OpenEXR_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenEXR::Iex")
endif()
if(OpenEXR_FOUND AND NOT TARGET OpenEXR::OpenEXR)
    add_library(OpenEXR::OpenEXR UNKNOWN IMPORTED)
    set_target_properties(OpenEXR::OpenEXR PROPERTIES
        IMPORTED_LOCATION "${OpenEXR_OpenEXR_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${OpenEXR_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenEXR::IlmThread;Imath")
endif()
if(OpenEXR_FOUND AND NOT TARGET OpenEXR::OpenEXRUtil)
    add_library(OpenEXR::OpenEXRUtil UNKNOWN IMPORTED)
    set_target_properties(OpenEXR::OpenEXRUtil PROPERTIES
        IMPORTED_LOCATION "${OpenEXR_OpenEXRUtil_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${OpenEXR_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OpenEXR_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenEXR::OpenEXR")
endif()
if(OpenEXR_FOUND AND NOT TARGET OpenEXR)
    add_library(OpenEXR INTERFACE)
    target_link_libraries(OpenEXR INTERFACE OpenEXR::Iex)
    target_link_libraries(OpenEXR INTERFACE OpenEXR::IlmThread)
    target_link_libraries(OpenEXR INTERFACE OpenEXR::OpenEXR)
    target_link_libraries(OpenEXR INTERFACE OpenEXR::OpenEXRUtil)
endif()
