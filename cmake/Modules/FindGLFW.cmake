# Find the GLFW library.
#
# This module defines the following variables:
#
# * GLFW_INCLUDE_DIRS
# * GLFW_LIBRARIES
#
# This module defines the following imported targets:
#
# * GLFW::glfw
#
# This module defines the following interfaces:
#
# * GLFW

find_package(Threads REQUIRED)
find_package(OpenGL REQUIRED)
if(WIN32)
elseif(APPLE)
    find_library(CORE_VIDEO CoreVideo REQUIRED)
    find_library(IO_KIT IOKit REQUIRED)
    find_library(COCOA Cocoa REQUIRED)
    find_library(CARBON Carbon REQUIRED)
else()
    find_package(X11 REQUIRED)
endif()

find_path(GLFW_INCLUDE_DIR NAMES GLFW/glfw3.h)
set(GLFW_INCLUDE_DIRS ${GLFW_INCLUDE_DIR})
if(WIN32)
elseif(APPLE)
else()
    list(APPEND GLFW_INCLUDE_DIRS ${X11_INCLUDE_DIR})
endif()
list(APPEND GLFW_INCLUDE_DIRS ${OPENGL_INCLUDE_DIR})

find_library(GLFW_LIBRARY NAMES glfw3dll glfw3 glfw)
set(GLFW_LIBRARIES ${GLFW_LIBRARY})
if(WIN32)
elseif(APPLE)
    list(APPEND GLFW_LIBRARIES ${CORE_VIDEO} ${IO_KIT} ${COCOA} ${CARBON})
else()
    list(APPEND GLFW_LIBRARIES ${X11_LIBRARIES})
endif()
list(APPEND GLFW_LIBRARIES ${OPENGL_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    GLFW
    REQUIRED_VARS GLFW_INCLUDE_DIR GLFW_LIBRARY)
mark_as_advanced(GLFW_INCLUDE_DIR GLFW_LIBRARY)

if(GLFW_FOUND AND NOT TARGET GLFW::glfw)
    add_library(GLFW::glfw UNKNOWN IMPORTED)
    set(GLFW_INCLUDE_DIRECTORIES ${GLFW_INCLUDE_DIR})
    set(GLFW_LINK_LIBRARIES)
    if(WIN32)
        list(APPEND GLFW_LINK_LIBRARIES OpenGL::GL)
    elseif(APPLE)
	    list(APPEND GLFW_LINK_LIBRARIES ${CORE_VIDEO} ${IO_KIT} ${COCOA} ${CARBON} OpenGL::GL)
    else()
		list(APPEND GLFW_INCLUDE_DIRECTORIES ${X11_INCLUDE_DIR})
	    list(APPEND GLFW_LINK_LIBRARIES ${X11_LIBRARIES} OpenGL::GL ${CMAKE_THREAD_LIBS_INIT} ${CMAKE_DL_LIBS})
    endif()
    set_target_properties(GLFW::glfw PROPERTIES
        IMPORTED_LOCATION "${GLFW_LIBRARY}"
        INTERFACE_COMPILE_DEFINITIONS GLFW_FOUND
        INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIRECTORIES}"
        INTERFACE_LINK_LIBRARIES "${GLFW_LINK_LIBRARIES}")
endif()
if(GLFW_FOUND AND NOT TARGET GLFW)
    add_library(GLFW INTERFACE)
    target_link_libraries(GLFW INTERFACE GLFW::glfw)
endif()
