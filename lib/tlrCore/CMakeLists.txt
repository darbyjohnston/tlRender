set(HEADERS
    AVIO.h
    AVIOInline.h
    Assert.h
    BBox.h
    BBoxInline.h
    Cache.h
    CacheInline.h
    Color.h
    ColorInline.h
    Cineon.h
    DPX.h
    Error.h
    File.h
    FileIO.h
    Image.h
    ImageInline.h
    ListObserver.h
    ListObserverInline.h
    MapObserver.h
    MapObserverInline.h
    Math.h
    MathInline.h
    Matrix.h
    MatrixInline.h
    Memory.h
    MemoryInline.h
    Observer.h
    Range.h
    RangeInline.h
    SequenceIO.h
    String.h
    StringFormat.h
    StringFormatInline.h
    Time.h
    Timeline.h
    TimelineInline.h
    TimelinePlayer.h
    TimelinePlayerInline.h
    Util.h
    ValueObserver.h
    ValueObserverInline.h
    Vector.h
    VectorInline.h)
set(SOURCE
    AVIO.cpp
    Assert.cpp
    CineonRead.cpp
    CineonWrite.cpp
    Cineon.cpp
    DPXRead.cpp
    DPXWrite.cpp
    DPX.cpp
    Error.cpp
    File.cpp
    FileIO.cpp
    Image.cpp
    Memory.cpp
    SequenceIO.cpp
    String.cpp
    StringFormat.cpp
    Time.cpp
    Timeline.cpp
    TimelinePlayer.cpp)
if (WIN32)
    set(SOURCE
        ${SOURCE}
        ErrorWin32.cpp
        FileIOWin32.cpp
        FileWin32.cpp
        TimeWin32.cpp)
else()
    set(SOURCE
        ${SOURCE}
        FileIOUnix.cpp
        FileUnix.cpp
        TimeUnix.cpp)
endif()

set(tlrCore_LIBRARIES OTIO FSeq)
if(TLR_ENABLE_PYTHON)
    list(APPEND tlrCore_LIBRARIES Python3::Python)
endif()
if(JPEG_FOUND)
    set(HEADERS ${HEADERS} JPEG.h)
    set(SOURCE ${SOURCE} JPEG.cpp JPEGRead.cpp JPEGWrite.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} JPEG)
endif()
if(TIFF_FOUND)
    set(HEADERS ${HEADERS} TIFF.h)
    set(SOURCE ${SOURCE} TIFF.cpp TIFFRead.cpp TIFFWrite.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} TIFF)
endif()
if(PNG_FOUND)
    set(HEADERS ${HEADERS} PNG.h)
    set(SOURCE ${SOURCE} PNG.cpp PNGRead.cpp PNGWrite.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} PNG)
endif()
if(OpenEXR_FOUND)
    set(HEADERS ${HEADERS} OpenEXR.h)
    set(SOURCE ${SOURCE} OpenEXR.cpp OpenEXRRead.cpp OpenEXRWrite.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} OpenEXR)
endif()
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpeg.h)
    set(SOURCE ${SOURCE} FFmpeg.cpp FFmpegRead.cpp FFmpegWrite.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} FFmpeg)
endif()
set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} IlmBase Threads::Threads)

add_library(tlrCore ${HEADERS} ${SOURCE})
target_link_libraries(tlrCore ${tlrCore_LIBRARIES})
set_target_properties(tlrCore PROPERTIES FOLDER lib)
