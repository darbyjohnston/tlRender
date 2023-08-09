set(HEADERS
    AudioClipItem.h
    AudioGapItem.h
    Edit.h
    IBasicItem.h
    IItem.h
    IOManager.h
    Init.h
    TimelineItem.h
    TimelineViewport.h
    TimelineWidget.h
    TrackItem.h
    TransitionItem.h
    VideoClipItem.h
    VideoGapItem.h)
set(HEADERS_PRIVATE)

set(SOURCE
    AudioClipItem.cpp
    AudioGapItem.cpp
    Edit.cpp
    IBasicItem.cpp
    IItem.cpp
    IOManager.cpp
    Init.cpp
    TimelineItem.cpp
    TimelineViewport.cpp
    TimelineWidget.cpp
    TrackItem.cpp
    TransitionItem.cpp
    VideoClipItem.cpp
    VideoGapItem.cpp)

add_library(tlTimelineUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlTimelineUI PUBLIC tlUI tlTimeline PRIVATE ${LIBRARIES_PRIVATE})
set_target_properties(tlTimelineUI PROPERTIES FOLDER lib)
set_target_properties(tlTimelineUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimelineUI
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineUI)
