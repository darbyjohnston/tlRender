set(HEADERS
    BBoxTest.h
    CacheTest.h
    ColorTest.h
    FileTest.h
    IOTest.h
    JPEGTest.h
    ListObserverTest.h
    MapObserverTest.h
    OpenEXRTest.h
    PNGTest.h
    TIFFTest.h
    ImageTest.h
    TimelineTest.h
    ValueObserverTest.h)
set(SOURCE
    BBoxTest.cpp
    CacheTest.cpp
    ColorTest.cpp
    FileTest.cpp
    IOTest.cpp
    JPEGTest.cpp
    ListObserverTest.cpp
    MapObserverTest.cpp
    OpenEXRTest.cpp
    PNGTest.cpp
    TIFFTest.cpp
    ImageTest.cpp
    TimelineTest.cpp
    ValueObserverTest.cpp)
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpegTest.h)
    set(SOURCE ${SOURCE} FFmpegTest.cpp)
endif()

add_library(tlrCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrCoreTest tlrTestLib)
set_target_properties(tlrCoreTest PROPERTIES FOLDER tests)
