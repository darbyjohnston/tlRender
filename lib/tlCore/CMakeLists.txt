set(HEADERS
    Audio.h
    AudioInline.h
    AudioResample.h
    AudioSystem.h
    Box.h
    BoxInline.h
    Context.h
    ContextInline.h
    File.h
    FileIO.h
    FileIOInline.h
    FileInfo.h
    FileInfoInline.h
    FileInfoPrivate.h
    FileLogSystem.h
    FontSystem.h
    FontSystemInline.h
    HDR.h
    HDRInline.h
    ICoreSystem.h
    ICoreSystemInline.h
    ISystem.h
    Image.h
    ImageInline.h
    LRUCache.h
    LRUCacheInline.h
    ListObserver.h
    ListObserverInline.h
    LogSystem.h
    MapObserver.h
    MapObserverInline.h
    Math.h
    MathInline.h
    Matrix.h
    MatrixInline.h
    Memory.h
    MemoryInline.h
    Mesh.h
    MeshInline.h
    Observer.h
    Path.h
    PathInline.h
    Random.h
    RandomInline.h
    Range.h
    RangeInline.h
    Size.h
    SizeInline.h
    Time.h
    TimeInline.h
    Timer.h
    Util.h
    ValueObserver.h
    ValueObserverInline.h
    Vector.h
    VectorInline.h)

set(SOURCE
    Audio.cpp
    AudioResample.cpp
    AudioSystem.cpp
    Box.cpp
    Context.cpp
    FileIO.cpp
    FileInfo.cpp
    FileLogSystem.cpp
    FontSystem.cpp
    HDR.cpp
    ICoreSystem.cpp
    ISystem.cpp
    Image.cpp
    LogSystem.cpp
    Matrix.cpp
    Memory.cpp
    Mesh.cpp
    Path.cpp
    Random.cpp
    Range.cpp
    Size.cpp
    Time.cpp
    Timer.cpp
    Vector.cpp)
if (WIN32)
    list(APPEND SOURCE
        FileIOWin32.cpp
        FileInfoWin32.cpp
        FileWin32.cpp
        PathWin32.cpp
        TimeWin32.cpp)
else()
    list(APPEND SOURCE
        FileIOUnix.cpp
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
