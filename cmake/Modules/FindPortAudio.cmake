# Find the Port library.
#
# This module defines the following variables:
#
# * portaudio_FOUND
# * portaudio_INCLUDE_DIRS
# * portaudio_LIBRARIES
#
# This module defines the following imported targets:
#
# * portaudio::portaudio
#
# This module defines the following interfaces:
#
# * portaudio

find_path(portaudio_INCLUDE_DIR NAMES portaudio.h)
set(portaudio_INCLUDE_DIRS ${portaudio_INCLUDE_DIR})

find_library(portaudio_LIBRARY NAMES portaudio)
set(portaudio_LIBRARIES ${portaudio_LIBRARY} ${portaudio_LINK_LIBRARIES})
if(WIN32)
elseif(APPLE)
    find_library(CORE_FOUNDATION CoreFoundation REQUIRED)
    find_library(CORE_AUDIO CoreAudio)
    find_library(AUDIO_TOOLBOX AudioToolbox)
    list(APPEND portaudio_LIBRARIES ${AUDIO_TOOLBOX} ${CORE_AUDIO} ${CORE_FOUNDATION})
else()
    find_package(Threads REQUIRED)
    find_package(X11 REQUIRED)
    find_package(ALSA REQUIRED)
    set(portaudio_RPM_PACKAGE_REQUIRES "alsa-lib")
    set(portaudio_DEBIAN_PACKAGE_DEPENDS "libasound2")
    find_library(PULSE_LIB pulse)
    find_library(PULSESIMPLE_LIB pulse-simple)
    if(PULSE_LIB AND PULSESIMPLE_LIB)
	string(APPEND portaudio_RPM_PACKAGE_REQUIRES ", pulseaudio-libs")
	string(APPEND portaudio_DEBIAN_PACKAGE_DEPENDS ", libpulse0")
	list(APPEND portaudio_INCLUDE_DIRS ${PULSE_INCLUDE_DIR} ${PULSESIMPLE_INCLUDE_DIR})
	list(APPEND portaudio_LIBRARIES ${PULSE_LIB} ${PULSESIMPLE_LIB})
    endif()
    find_library(JACK_LIB jack)
    if(JACK_LIB)
	list(APPEND portaudio_INCLUDE_DIRS ${JACK_INCLUDE_DIR})
	list(APPEND portaudio_LIBRARIES ${JACK_LIB})
    endif()
    list(APPEND portaudio_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
endif()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
    PortAudio
    REQUIRED_VARS portaudio_INCLUDE_DIR portaudio_LIBRARY)
mark_as_advanced(portaudio_INCLUDE_DIR portaudio_LIBRARY)

if(portaudio_FOUND AND NOT TARGET portaudio::portaudio)
    add_library(portaudio::portaudio UNKNOWN IMPORTED)
    set(portaudio_INTERFACE_INCLUDE_DIRECTORIES ${portaudio_INCLUDE_DIR})
    set(portaudio_INTERFACE_LINK_LIBRARIES)
    if(WIN32)
    elseif(APPLE)
		list(APPEND portaudio_INTERFACE_LINK_LIBRARIES ${AUDIO_TOOLBOX} ${CORE_AUDIO} ${CORE_FOUNDATION})
    else()
		list(APPEND portaudio_INTERFACE_INCLUDE_DIRECTORIES ${ALSA_INCLUDE_DIR})
		list(APPEND portaudio_INTERFACE_LINK_LIBRARIES ${ALSA_LIBRARY})
	if(PULSE_LIB AND PULSESIMPLE_LIB)
		list(APPEND portaudio_INTERFACE_INCLUDE_DIRECTORIES ${PULSE_INCLUDE_DIR} ${PULSESIMPLE_INCLUDE_DIR})
		list(APPEND portaudio_INTERFACE_LINK_LIBRARIES ${PULSE_LIB} ${PULSESIMPLE_LIB})
	endif()
	if(JACK_LIB)
		list(APPEND portaudio_INTERFACE_INCLUDE_DIRECTORIES ${JACK_INCLUDE_DIR})
		list(APPEND portaudio_INTERFACE_LINK_LIBRARIES ${JACK_LIB})
	endif()
		list(APPEND portaudio_INTERFACE_LINK_LIBRARIES ${CMAKE_THREAD_LIBS_INIT})
    endif()
    set_target_properties(portaudio::portaudio PROPERTIES
	IMPORTED_LOCATION "${portaudio_LIBRARY}"
	INTERFACE_COMPILE_DEFINITIONS portaudio_FOUND
	INTERFACE_INCLUDE_DIRECTORIES "${portaudio_INTERFACE_INCLUDE_DIRECTORIES}"
	INTERFACE_LINK_LIBRARIES "${portaudio_INTERFACE_LINK_LIBRARIES}")
endif()
if(portaudio_FOUND AND NOT TARGET portaudio)
    add_library(portaudio INTERFACE)
    target_link_libraries(portaudio INTERFACE portaudio::portaudio)
endif()
