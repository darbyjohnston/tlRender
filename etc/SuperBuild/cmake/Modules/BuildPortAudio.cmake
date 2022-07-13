include(ExternalProject)

set(portaudio_GIT_REPOSITORY "https://github.com/PortAudio/portaudio.git")
set(portaudio_GIT_TAG "v19.7.0") # 2020/06/07

set(portaudio_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_DEBUG_POSTFIX=
    -DPORTAUDIO_BUILD_TESTING=FALSE
    -DPORTAUDIO_STATIC_MSVCRT=FALSE)
if(BUILD_SHARED_LIBS)
    list(APPEND portaudio_ARGS -DPORTAUDIO_BUILD_SHARED_LIBS=TRUE)
else()
    list(APPEND portaudio_ARGS -DPORTAUDIO_BUILD_STATIC_LIBS=TRUE)
endif()
if(APPLE)
    list(APPEND portaudio_ARGS PORTAUDIO_API_JACK=OFF)
endif()

ExternalProject_Add(
    PortAudio
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/PortAudio
    GIT_REPOSITORY ${portaudio_GIT_REPOSITORY}
    GIT_TAG ${portaudio_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${portaudio_ARGS})
