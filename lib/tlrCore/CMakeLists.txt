set(HEADERS
    Assert.h
    BBox.h
    BBoxInline.h
    Cache.h
    CacheInline.h
    Color.h
    ColorInline.h
    Error.h
    File.h
    IO.h
    IOInline.h
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
    Observer.h
    Range.h
    RangeInline.h
    SequenceIO.h
    String.h
    StringFormat.h
    Time.h
    Timeline.h
    TimelineInline.h
    Util.h
    ValueObserver.h
    ValueObserverInline.h
    Vector.h
    VectorInline.h)
set(SOURCE
    Assert.cpp
    File.cpp
    IO.cpp
    Image.cpp
    SequenceIO.cpp
    String.cpp
    StringFormat.cpp
    Time.cpp
    Timeline.cpp
    Util.cpp)

set(tlrCore_LIBRARIES OTIO FSeq)
if(TLR_ENABLE_PYTHON)
    list(APPEND tlrCore_LIBRARIES Python3::Python)
endif()
if(JPEG_FOUND)
    set(HEADERS ${HEADERS} JPEG.h)
    set(SOURCE ${SOURCE} JPEG.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} JPEG)
endif()
if(OpenEXR_FOUND)
    set(HEADERS ${HEADERS} OpenEXR.h)
    set(SOURCE ${SOURCE} OpenEXR.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} OpenEXR)
endif()
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpeg.h)
    set(SOURCE ${SOURCE} FFmpeg.cpp)
    set(tlrCore_LIBRARIES ${tlrCore_LIBRARIES} FFmpeg)
endif()

add_library(tlrCore ${HEADERS} ${SOURCE})
target_link_libraries(tlrCore ${tlrCore_LIBRARIES})
set_target_properties(tlrCore PROPERTIES FOLDER lib)
