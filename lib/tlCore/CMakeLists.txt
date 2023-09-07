set(HEADERS
    Assert.h
    Audio.h
    AudioConvert.h
    AudioInline.h
    AudioSystem.h
    Box.h
    BoxInline.h
    Color.h
    ColorInline.h
    Context.h
    ContextInline.h
    Error.h
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
    OS.h
    Observer.h
    Path.h
    PathInline.h
    Random.h
    RandomInline.h
    Range.h
    RangeInline.h
    Size.h
    SizeInline.h
    String.h
    StringFormat.h
    StringFormatInline.h
    Time.h
    TimeInline.h
    Timer.h
    Util.h
    ValueObserver.h
    ValueObserverInline.h
    Vector.h
    VectorInline.h)

set(SOURCE
    Assert.cpp
    Audio.cpp
    AudioConvert.cpp
    AudioSystem.cpp
    Box.cpp
    Color.cpp
    Context.cpp
    Error.cpp
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
    OS.cpp
    Path.cpp
    Random.cpp
    Range.cpp
    Size.cpp
    String.cpp
    StringFormat.cpp
    Time.cpp
    Timer.cpp
    Vector.cpp)
if (WIN32)
    list(APPEND SOURCE
        ErrorWin32.cpp
        FileIOWin32.cpp
        FileInfoWin32.cpp
        FileWin32.cpp
        OSWin32.cpp
        PathWin32.cpp
        TimeWin32.cpp)
else()
    list(APPEND SOURCE
        FileIOUnix.cpp
        FileInfoUnix.cpp
        FileUnix.cpp
        OSUnix.cpp
        PathUnix.cpp
        TimeUnix.cpp)
endif()

set(LIBRARIES OTIO Imath::Imath)
set(LIBRARIES_PRIVATE FreeType nlohmann_json::nlohmann_json MINIZIP::minizip ZLIB)
if(TLRENDER_OCIO AND NOT "${TLRENDER_API}" STREQUAL "GLES_2")
    list(APPEND LIBRARIES OpenColorIO::OpenColorIO)
endif()
if(TLRENDER_AUDIO)
    list(APPEND LIBRARIES_PRIVATE RtAudio libsamplerate)
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
