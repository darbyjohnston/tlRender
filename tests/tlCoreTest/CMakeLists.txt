set(HEADERS
    AudioTest.h
    BBoxTest.h
	ColorConfigTest.h
    ColorTest.h
    ContextTest.h
    ErrorTest.h
    FileIOTest.h
    FileTest.h
	HDRTest.h
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
    ValueObserverTest.h
	VectorTest.h)

set(SOURCE
    AudioTest.cpp
    BBoxTest.cpp
	ColorConfigTest.cpp
    ColorTest.cpp
    ContextTest.cpp
    ErrorTest.cpp
    FileIOTest.cpp
    FileTest.cpp
	HDRTest.cpp
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
    ValueObserverTest.cpp
	VectorTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
