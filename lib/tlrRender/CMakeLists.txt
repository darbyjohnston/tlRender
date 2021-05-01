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
    Math.h
    MathInline.h
    Matrix.h
    MatrixInline.h
    Range.h
    RangeInline.h
    String.h
    StringFormat.h
    Time.h
    Timeline.h
    TimelineInline.h
    Util.h
    Vector.h
    VectorInline.h)
set(SOURCE
    Assert.cpp
    File.cpp
    IO.cpp
    Image.cpp
    String.cpp
    StringFormat.cpp
    Time.cpp
    Timeline.cpp
    Util.cpp)

set(tlrRender_LIBRARIES OTIO FSeq)
if(TLR_ENABLE_PYTHON)
    list(APPEND tlrRender_LIBRARIES Python3::Python)
endif()
if(JPEG_FOUND)
    set(HEADERS ${HEADERS} JPEG.h)
    set(SOURCE ${SOURCE} JPEG.cpp)
    set(tlrRender_LIBRARIES ${tlrRender_LIBRARIES} JPEG)
endif()
if(OpenEXR_FOUND)
    set(HEADERS ${HEADERS} OpenEXR.h)
    set(SOURCE ${SOURCE} OpenEXR.cpp)
    set(tlrRender_LIBRARIES ${tlrRender_LIBRARIES} OpenEXR)
endif()
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpeg.h)
    set(SOURCE ${SOURCE} FFmpeg.cpp)
    set(tlrRender_LIBRARIES ${tlrRender_LIBRARIES} FFmpeg)
endif()

add_library(tlrRender ${HEADERS} ${SOURCE})
target_link_libraries(tlrRender ${tlrRender_LIBRARIES})
set_target_properties(tlrRender PROPERTIES FOLDER lib)
