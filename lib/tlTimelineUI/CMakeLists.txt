set(HEADERS
    AudioClipItem.h
    GapItem.h
    IBasicItem.h
    IItem.h
    Init.h
    ThumbnailSystem.h
    TimeEdit.h
    TimeLabel.h
    TimelineItem.h
    TimelineWidget.h
    TransitionItem.h
    VideoClipItem.h
    Viewport.h
    Window.h)
set(HEADERS_PRIVATE
    TimelineItemPrivate.h)

set(SOURCE
    AudioClipItem.cpp
    GapItem.cpp
    IBasicItem.cpp
    IItem.cpp
    Init.cpp
    ThumbnailSystem.cpp
    TimeEdit.cpp
    TimeLabel.cpp
    TimelineItem.cpp
    TimelineWidget.cpp
    TransitionItem.cpp
    VideoClipItem.cpp
    Viewport.cpp
    Window.cpp)

add_library(tlTimelineUI ${HEADERS} ${HEADERS_PRIVATE} ${SOURCE})
target_link_libraries(tlTimelineUI PUBLIC tlTimelineGL ftk::ftkUI)
set_target_properties(tlTimelineUI PROPERTIES FOLDER lib)
set_target_properties(tlTimelineUI PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlTimelineUI
    EXPORT tlTimelineUITargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineUI)
install(
    EXPORT tlTimelineUITargets
    FILE tlTimelineUITargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
