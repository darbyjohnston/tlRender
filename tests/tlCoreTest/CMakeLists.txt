set(HEADERS
    AudioTest.h
    BoxTest.h
    FileInfoTest.h
    FileTest.h
    FontSystemTest.h
    HDRTest.h
    ImageTest.h
    LRUCacheTest.h
    MathTest.h
    MatrixTest.h
    MemoryTest.h
    MeshTest.h
    PathTest.h
    RangeTest.h
    SizeTest.h
    TimeTest.h
    VectorTest.h)

set(SOURCE
    AudioTest.cpp
    BoxTest.cpp
    FileInfoTest.cpp
    FileTest.cpp
    FontSystemTest.cpp
    HDRTest.cpp
    ImageTest.cpp
    LRUCacheTest.cpp
    MathTest.cpp
    MatrixTest.cpp
    MemoryTest.cpp
    MeshTest.cpp
    PathTest.cpp
    RangeTest.cpp
    SizeTest.cpp
    TimeTest.cpp
    VectorTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
