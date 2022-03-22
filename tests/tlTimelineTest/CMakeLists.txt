set(HEADERS
	IRenderTest.h
    TimelinePlayerTest.h
    TimelineTest.h
    TimelineUtilTest.h)
set(SOURCE
	IRenderTest.cpp
    TimelinePlayerTest.cpp
    TimelineTest.cpp
    TimelineUtilTest.cpp)

add_library(tlTimelineTest ${SOURCE} ${HEADERS})
target_link_libraries(tlTimelineTest tlTestLib tlTimeline)
set_target_properties(tlTimelineTest PROPERTIES FOLDER tests)
