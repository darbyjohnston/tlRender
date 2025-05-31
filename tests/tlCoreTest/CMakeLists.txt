set(HEADERS
    AudioTest.h
    FileInfoTest.h
    HDRTest.h
    PathTest.h
    TimeTest.h
    URLTest.h)

set(SOURCE
    AudioTest.cpp
    FileInfoTest.cpp
    HDRTest.cpp
    PathTest.cpp
    TimeTest.cpp
    URLTest.cpp)

add_library(tlCoreTest ${SOURCE} ${HEADERS})
target_link_libraries(tlCoreTest tlTestLib)
set_target_properties(tlCoreTest PROPERTIES FOLDER tests)
