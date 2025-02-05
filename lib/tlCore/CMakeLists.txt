set(HEADERS
    Audio.h
    AudioInline.h
    AudioResample.h
    AudioSystem.h
    File.h
    FileInfo.h
    FileInfoInline.h
    FileInfoPrivate.h
    FileLogSystem.h
    HDR.h
    HDRInline.h
    ISystem.h
    Init.h
    Path.h
    PathInline.h
    Time.h
    TimeInline.h
    Timer.h)

set(SOURCE
    Audio.cpp
    AudioResample.cpp
    AudioSystem.cpp
    FileInfo.cpp
    FileLogSystem.cpp
    HDR.cpp
    ISystem.cpp
    Init.cpp
    Path.cpp
    Time.cpp
    Timer.cpp)
if (WIN32)
    list(APPEND SOURCE
        FileInfoWin32.cpp
        FileWin32.cpp
        PathWin32.cpp
        TimeWin32.cpp)
else()
    list(APPEND SOURCE
        FileInfoUnix.cpp
        FileUnix.cpp
        PathUnix.cpp
        TimeUnix.cpp)
endif()

set(LIBRARIES dtk::dtkCore OTIO Imath::Imath nlohmann_json::nlohmann_json)
set(LIBRARIES_PRIVATE Freetype::Freetype MINIZIP::minizip ZLIB)
if(TLRENDER_OCIO AND NOT "${TLRENDER_API}" STREQUAL "GLES_2")
    list(APPEND LIBRARIES OpenColorIO::OpenColorIO)
endif()
if(TLRENDER_SDL2)
    list(APPEND LIBRARIES_PRIVATE SDL2::SDL2-static)
endif()
if(TLRENDER_SDL3)
    list(APPEND LIBRARIES_PRIVATE SDL3::SDL3-static)
endif()
if(TLRENDER_FFMPEG)
    list(APPEND LIBRARIES_PRIVATE FFmpeg::swresample)
endif()
if(TLRENDER_PYTHON)
    list(APPEND LIBRARIES_PRIVATE Python3::Python)
endif()
list(APPEND LIBRARIES_PRIVATE Threads::Threads)

add_library(tlCore ${HEADERS} ${SOURCE})
target_link_libraries(tlCore PUBLIC ${LIBRARIES} PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlCore PROPERTIES FOLDER lib)
set_target_properties(tlCore PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlCore
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlCore)
