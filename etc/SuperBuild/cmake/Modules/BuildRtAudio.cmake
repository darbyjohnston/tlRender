include(ExternalProject)

set(RtAudio_GIT_REPOSITORY "https://github.com/thestk/rtaudio.git")
set(RtAudio_GIT_TAG "5.2.0")

set(RtAudio_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_DEBUG_POSTFIX=
    -DRTAUDIO_BUILD_TESTING=FALSE
    -DRTAUDIO_STATIC_MSVCRT=FALSE
    -DRTAUDIO_API_JACK=OFF)
if(BUILD_SHARED_LIBS)
    list(APPEND RtAudio_ARGS -DRTAUDIO_BUILD_SHARED_LIBS=TRUE)
else()
    list(APPEND RtAudio_ARGS -DRTAUDIO_BUILD_STATIC_LIBS=TRUE)
endif()
if(NOT WIN32 AND NOT APPLE)
    list(APPEND RtAudio_ARGS -DRTAUDIO_API_PULSE=ON)
endif()

ExternalProject_Add(
    RtAudio
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/RtAudio
    GIT_REPOSITORY ${RtAudio_GIT_REPOSITORY}
    GIT_TAG ${RtAudio_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${RtAudio_ARGS})
