set(HEADERS
    ColorConfigOptionsTest.h
    IRenderTest.h
    LUTOptionsTest.h
    TimelinePlayerTest.h
    TimelineTest.h
    TimelineUtilTest.h)

set(SOURCE
    ColorConfigOptionsTest.cpp
    IRenderTest.cpp
    LUTOptionsTest.cpp
    TimelinePlayerTest.cpp
    TimelineTest.cpp
    TimelineUtilTest.cpp)

add_library(tlTimelineTest ${SOURCE} ${HEADERS})
target_link_libraries(tlTimelineTest tlTestLib tlTimeline)
set_target_properties(tlTimelineTest PROPERTIES FOLDER tests)
