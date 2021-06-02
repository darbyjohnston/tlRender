include(ExternalProject)

# Tag: 2020/06/07
set(RtAudio_GIT_TAG d7f12763c55795ef8a71a9b589b39e7be01db7b2)

set(RtAudio_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DCMAKE_DEBUG_POSTFIX=
    -DRTAUDIO_BUILD_TESTING=FALSE
    -DRTAUDIO_STATIC_MSVCRT=FALSE)
if(BUILD_SHARED_LIBS)
    set(RtAudio_ARGS ${RtAudio_ARGS} -DRTAUDIO_BUILD_SHARED_LIBS=TRUE)
else()
    set(RtAudio_ARGS ${RtAudio_ARGS} -DRTAUDIO_BUILD_STATIC_LIBS=TRUE)
endif()
if(CMAKE_CXX_STANDARD)
    set(RtAudio_ARGS ${RtAudio_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
endif()

ExternalProject_Add(
    RtAudio
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/RtAudio
    GIT_REPOSITORY https://github.com/thestk/rtaudio.git
    GIT_TAG ${RtAudio_GIT_TAG}
    CMAKE_ARGS ${RtAudio_ARGS})
