set(HEADERS
    Audio.h
    BackgroundOptions.h
    ColorOptions.h
    CompareOptions.h
    DisplayOptions.h
    Edit.h
    ForegroundOptions.h
    IRender.h
    Init.h
    MemoryReference.h
    Player.h
    PlayerOptions.h
    RenderUtil.h
    TimeUnits.h
    Timeline.h
    TimelineOptions.h
    Transition.h
    Util.h
    Video.h)
set(PRIVATE_HEADERS
    PlayerPrivate.h
    TimelinePrivate.h)

set(SOURCE
    Audio.cpp
    BackgroundOptions.cpp
    ColorOptions.cpp
    CompareOptions.cpp
    DisplayOptions.cpp
    Edit.cpp
    ForegroundOptions.cpp
    IRender.cpp
    Init.cpp
    MemoryReference.cpp
    Player.cpp
    PlayerAudio.cpp
    PlayerOptions.cpp
    PlayerPrivate.cpp
    RenderUtil.cpp
    TimeUnits.cpp
    Timeline.cpp
    TimelineCreate.cpp
    TimelineOptions.cpp
    TimelinePrivate.cpp
    Transition.cpp
    Util.cpp
    Video.cpp)

add_library(tlTimeline ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
