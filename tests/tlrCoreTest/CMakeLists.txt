set(HEADERS
    AVIOTest.h
    BBoxTest.h
    CineonTest.h
    ColorTest.h
    DPXTest.h
    ErrorTest.h
    FileIOTest.h
    FileTest.h
    ImageTest.h
    LRUCacheTest.h
    ListObserverTest.h
    MapObserverTest.h
    MathTest.h
    MatrixTest.h
    MemoryTest.h
    PathTest.h
    RangeTest.h
    StringTest.h
    StringFormatTest.h
    TimeTest.h
    TimelinePlayerTest.h
    TimelineTest.h
    ValueObserverTest.h
    VectorTest.h)
set(SOURCE
    AVIOTest.cpp
    BBoxTest.cpp
    CineonTest.cpp
    ColorTest.cpp
    DPXTest.cpp
    ErrorTest.cpp
    FileIOTest.cpp
    FileTest.cpp
    ImageTest.cpp
    LRUCacheTest.cpp
    ListObserverTest.cpp
    MapObserverTest.cpp
    MathTest.cpp
    MatrixTest.cpp
    MemoryTest.cpp
    PathTest.cpp
    RangeTest.cpp
    StringTest.cpp
    StringFormatTest.cpp
    TimeTest.cpp
    TimelinePlayerTest.cpp
    TimelineTest.cpp
    ValueObserverTest.cpp
    VectorTest.cpp)
if(FFmpeg_FOUND)
    list(APPEND HEADERS FFmpegTest.h)
    list(APPEND SOURCE FFmpegTest.cpp)
endif()
if(JPEG_FOUND)
    list(APPEND HEADERS JPEGTest.h)
    list(APPEND SOURCE JPEGTest.cpp)
endif()
if(PNG_FOUND)
    list(APPEND HEADERS PNGTest.h)
    list(APPEND SOURCE PNGTest.cpp)
endif()
if(OpenEXR_FOUND)
    list(APPEND HEADERS OpenEXRTest.h)
    list(APPEND SOURCE OpenEXRTest.cpp)
endif()
if(TIFF_FOUND)
    list(APPEND HEADERS TIFFTest.h)
    list(APPEND SOURCE TIFFTest.cpp)
endif()

add_library(tlrCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrCoreTest tlrTestLib)
set_target_properties(tlrCoreTest PROPERTIES FOLDER tests)
