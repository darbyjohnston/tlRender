set(HEADERS
    AudioTest.h
    FileInfoTest.h
    FileTest.h
    HDRTest.h
    PathTest.h
    TimeTest.h)

set(SOURCE
    AudioTest.cpp
    FileInfoTest.cpp
    FileTest.cpp
    HDRTest.cpp
    PathTest.cpp
    TimeTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
