set(HEADERS
    BBoxTest.h
    CacheTest.h
    ColorTest.h
    FileTest.h
    IOTest.h
    ImageTest.h
    JPEGTest.h
    ListObserverTest.h
    MapObserverTest.h
    MathTest.h
    MatrixTest.h
    MemoryTest.h
    OpenEXRTest.h
    PNGTest.h
    RangeTest.h
    StringTest.h
    StringFormatTest.h
    TIFFTest.h
    TimeTest.h
    TimelinePlayerTest.h
    TimelineTest.h
    ValueObserverTest.h)
set(SOURCE
    BBoxTest.cpp
    CacheTest.cpp
    ColorTest.cpp
    FileTest.cpp
    IOTest.cpp
    ImageTest.cpp
    JPEGTest.cpp
    ListObserverTest.cpp
    MapObserverTest.cpp
    MathTest.cpp
    MatrixTest.cpp
    MemoryTest.cpp
    OpenEXRTest.cpp
    PNGTest.cpp
    RangeTest.cpp
    StringTest.cpp
    StringFormatTest.cpp
    TIFFTest.cpp
    TimeTest.cpp
    TimelinePlayerTest.cpp
    TimelineTest.cpp
    ValueObserverTest.cpp)
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpegTest.h)
    set(SOURCE ${SOURCE} FFmpegTest.cpp)
endif()

add_library(tlrCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrCoreTest tlrTestLib)
set_target_properties(tlrCoreTest PROPERTIES FOLDER tests)
