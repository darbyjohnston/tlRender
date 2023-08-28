# Find the libjpeg library.
#
# This module defines the following variables:
#
# * JPEG_INCLUDE_DIRS
# * JPEG_LIBRARIES
#
# This module defines the following imported targets:
#
# * JPEG::JPEG
#
# This module defines the following interfaces:
#
# * JPEG

find_package(ZLIB)

find_path(JPEG_INCLUDE_DIR NAMES jpeglib.h)
set(JPEG_INCLUDE_DIRS
    ${JPEG_INCLUDE_DIR}
    ${ZLIB_INCLUDE_DIRS})

find_library(JPEG_LIBRARY NAMES jpeg-static jpeg)
set(JPEG_LIBRARIES
    ${JPEG_LIBRARY}
    ${ZLIB_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    JPEG
    REQUIRED_VARS JPEG_INCLUDE_DIR JPEG_LIBRARY)
mark_as_advanced(JPEG_INCLUDE_DIR JPEG_LIBRARY)

set(JPEG_COMPILE_DEFINITIONS JPEG_FOUND)
if(NOT JPEG_SHARED_LIBS)
    list(APPEND JPEG_COMPILE_DEFINITIONS JPEG_STATIC)
endif()

if(JPEG_FOUND AND NOT TARGET JPEG::JPEG)
    add_library(JPEG::JPEG UNKNOWN IMPORTED)
    set_target_properties(JPEG::JPEG PROPERTIES
        IMPORTED_LOCATION "${JPEG_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS "${JPEG_COMPILE_DEFINITIONS}"
        INTERFACE_INCLUDE_DIRECTORIES "${JPEG_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "ZLIB")
endif()
if(JPEG_FOUND AND NOT TARGET JPEG)
    add_library(JPEG INTERFACE)
    target_link_libraries(JPEG INTERFACE JPEG::JPEG)
endif()
