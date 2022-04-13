set(HEADERS
    AudioTest.h
    BBoxTest.h
    ColorTest.h
    ContextTest.h
    ErrorTest.h
    FileIOTest.h
    FileTest.h
    ImageTest.h
    LRUCacheTest.h
    ListObserverTest.h
    MapObserverTest.h
    MathTest.h
    MemoryTest.h
    PathTest.h
    RangeTest.h
    StringTest.h
    StringFormatTest.h
    TimeTest.h
    ValueObserverTest.h)

set(SOURCE
    AudioTest.cpp
    BBoxTest.cpp
    ColorTest.cpp
    ContextTest.cpp
    ErrorTest.cpp
    FileIOTest.cpp
    FileTest.cpp
    ImageTest.cpp
    LRUCacheTest.cpp
    ListObserverTest.cpp
    MapObserverTest.cpp
    MathTest.cpp
    MemoryTest.cpp
    PathTest.cpp
    RangeTest.cpp
    StringTest.cpp
    StringFormatTest.cpp
    TimeTest.cpp
    ValueObserverTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
