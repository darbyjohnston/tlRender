set(HEADERS
    ColorConfigOptionsTest.h
    EditTest.h
    IRenderTest.h
    LUTOptionsTest.h
    PlayerTest.h
    TimelineTest.h
    UtilTest.h)

set(SOURCE
    ColorConfigOptionsTest.cpp
    EditTest.cpp
    IRenderTest.cpp
    LUTOptionsTest.cpp
    PlayerTest.cpp
    TimelineTest.cpp
    UtilTest.cpp)

add_library(tlTimelineTest ${SOURCE} ${HEADERS})
target_link_libraries(tlTimelineTest tlTestLib tlTimeline)
set_target_properties(tlTimelineTest PROPERTIES FOLDER tests)
