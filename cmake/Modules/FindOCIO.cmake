# Find the OpenColorIO library.
#
# This module defines the following variables:
#
# * OCIO_FOUND
# * OCIO_COMPILE_DEFINITIONS
# * OCIO_INCLUDE_DIRS
# * OCIO_LIBRARIES
#
# This module defines the following imported targets:
#
# * OCIO::OpenColorIO
#
# This module defines the following interfaces:
#
# * OCIO

find_package(Imath)

find_path(OCIO_INCLUDE_DIR NAMES OpenColorIO/OpenColorIO.h)
set(OCIO_INCLUDE_DIRS
    ${OCIO_INCLUDE_DIR}
    ${Imath_INCLUDE_DIRS})

find_library(OCIO_LIBRARY NAMES OpenColorIO)
if(CMAKE_BUILD_TYPE MATCHES "^Debug$")
    find_library(OCIO_yaml_LIBRARY NAMES libyaml-cppmdd libyaml-cpp yaml-cpp)
    find_library(OCIO_pystring_LIBRARY NAMES pystring)
    find_library(OCIO_expat LIBRARY NAMES expatdMD expat)
else()
    find_library(OCIO_yaml_LIBRARY NAMES libyaml-cppmd libyaml-cpp yaml-cpp)
    find_library(OCIO_pystring_LIBRARY NAMES pystring)
    find_library(OCIO_expat LIBRARY NAMES expatMD expat)
endif()
set(OCIO_LIBRARIES
    ${OCIO_LIBRARY}
    ${OCIO_yaml_LIBRARY}
    ${OCIO_pystring_LIBRARY}
    ${OCIO_expat}
    ${Imath_LIBRARIES})
if(APPLE)
    list(APPEND OCIO_LIBRARIES "-framework ColorSync" "-framework CoreGraphics" "-framework IOKit" "-framework Foundation")
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    OCIO
    REQUIRED_VARS OCIO_INCLUDE_DIR OCIO_LIBRARY)
mark_as_advanced(OCIO_INCLUDE_DIR OCIO_LIBRARY)

set(OCIO_COMPILE_DEFINITIONS OCIO_FOUND)
if(NOT BUILD_SHARED_LIBS)
    list(APPEND OCIO_COMPILE_DEFINITIONS OpenColorIO_SKIP_IMPORTS)
endif()

if(OCIO_FOUND AND NOT TARGET OCIO::OpenColorIO)
    add_library(OCIO::OpenColorIO UNKNOWN IMPORTED)
    set(OCIO_INTERFACE_LINK_LIBRARIES ${OCIO_yaml_LIBRARY} ${OCIO_pystring_LIBRARY} ${OCIO_expat} Imath)
    if(APPLE)
        list(APPEND OCIO_INTERFACE_LINK_LIBRARIES "-framework ColorSync" "-framework CoreGraphics" "-framework IOKit" "-framework Foundation")
    endif()
    set_target_properties(OCIO::OpenColorIO PROPERTIES
        IMPORTED_LOCATION "${OCIO_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${OCIO_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${OCIO_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "${OCIO_INTERFACE_LINK_LIBRARIES}")
endif()
if(OCIO_FOUND AND NOT TARGET OCIO)
    add_library(OCIO INTERFACE)
    target_link_libraries(OCIO INTERFACE OCIO::OpenColorIO)
endif()
