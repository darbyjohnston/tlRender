set(HEADERS
    BBoxTest.h
    CacheTest.h
    ColorTest.h
    FileTest.h
    IOTest.h
    ImageTest.h
    TimelineTest.h)
set(SOURCE
    BBoxTest.cpp
    CacheTest.cpp
    ColorTest.cpp
    FileTest.cpp
    IOTest.cpp
    ImageTest.cpp
    TimelineTest.cpp)
if(FFmpeg_FOUND)
    set(HEADERS ${HEADERS} FFmpegTest.h)
    set(SOURCE ${SOURCE} FFmpegTest.cpp)
endif()

add_library(tlrCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrCoreTest tlrTestLib)
set_target_properties(tlrCoreTest PROPERTIES FOLDER tests)
