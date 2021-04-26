# Find the glad library.
#
# This module defines the following variables:
#
# * glad_INCLUDE_DIRS
# * glad_LIBRARIES
#
# This module defines the following imported targets:
#
# * glad::glad
#
# This module defines the following interfaces:
#
# * glad

find_package(OpenGL REQUIRED)

find_path(glad_INCLUDE_DIR NAMES glad.h)
set(glad_INCLUDE_DIRS
    ${glad_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIR})

find_library(glad_LIBRARY NAMES glad)
set(glad_LIBRARIES
    ${glad_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${CMAKE_DL_LIBS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    glad
    REQUIRED_VARS glad_INCLUDE_DIR glad_LIBRARY)
mark_as_advanced(glad_INCLUDE_DIR glad_LIBRARY)

if(glad_FOUND AND NOT TARGET glad::glad)
    add_library(glad::glad UNKNOWN IMPORTED)
    set_target_properties(glad::glad PROPERTIES
        IMPORTED_LOCATION "${glad_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${glad_INCLUDE_DIR}"
        INTERFACE_LINK_LIBRARIES "OpenGL::GL;${CMAKE_DL_LIBS}")
endif()
if(glad_FOUND AND NOT TARGET glad)
    add_library(glad INTERFACE)
    target_link_libraries(glad INTERFACE glad::glad)
endif()
