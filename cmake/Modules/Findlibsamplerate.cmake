# Find the libsamplerate library.
#
# This module defines the following variables:
#
# * libsamplerate_FOUND
# * libsamplerate_INCLUDE_DIRS
# * libsamplerate_LIBRARIES
#
# This module defines the following imported targets:
#
# * libsamplerate::libsamplerate
#
# This module defines the following interfaces:
#
# * libsamplerate

find_path(libsamplerate_INCLUDE_DIR NAMES samplerate.h)
set(libsamplerate_INCLUDE_DIRS ${libsamplerate_INCLUDE_DIR})

find_library(libsamplerate_LIBRARY NAMES samplerate)
set(libsamplerate_LIBRARIES ${libsamplerate_LIBRARY} ${libsamplerate_LINK_LIBRARIES})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    libsamplerate
    REQUIRED_VARS libsamplerate_INCLUDE_DIR libsamplerate_LIBRARY)
mark_as_advanced(libsamplerate_INCLUDE_DIR libsamplerate_LIBRARY)

if(libsamplerate_FOUND AND NOT TARGET libsamplerate::libsamplerate)
    add_library(libsamplerate::libsamplerate UNKNOWN IMPORTED)
    set_target_properties(libsamplerate::libsamplerate PROPERTIES
        IMPORTED_LOCATION "${libsamplerate_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS libsamplerate_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${libsamplerate_INCLUDE_DIR}")
endif()
if(libsamplerate_FOUND AND NOT TARGET libsamplerate)
    add_library(libsamplerate INTERFACE)
    target_link_libraries(libsamplerate INTERFACE libsamplerate::libsamplerate)
endif()
