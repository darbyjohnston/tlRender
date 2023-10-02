set(HEADERS
    ColorConfigOptionsTest.h
    CompareOptionsTest.h
    DisplayOptionsTest.h
    EditTest.h
    IRenderTest.h
    ImageOptionsTest.h
    LUTOptionsTest.h
    MemoryReferenceTest.h
    PlayerOptionsTest.h
    PlayerTest.h
    TimelineTest.h
    UtilTest.h)

set(SOURCE
    ColorConfigOptionsTest.cpp
    CompareOptionsTest.cpp
    DisplayOptionsTest.cpp
    EditTest.cpp
    IRenderTest.cpp
    ImageOptionsTest.cpp
    LUTOptionsTest.cpp
    MemoryReferenceTest.cpp
    PlayerOptionsTest.cpp
    PlayerTest.cpp
    TimelineTest.cpp
    UtilTest.cpp)

add_library(tlTimelineTest ${SOURCE} ${HEADERS})
target_link_libraries(tlTimelineTest tlTestLib tlTimeline)
set_target_properties(tlTimelineTest PROPERTIES FOLDER tests)
