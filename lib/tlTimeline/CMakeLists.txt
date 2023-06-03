set(HEADERS
    Audio.h
    AudioInline.h
    ColorConfigOptions.h
    ColorConfigOptionsInline.h
    CompareOptions.h
    CompareOptionsInline.h
    DisplayOptions.h
    DisplayOptionsInline.h
    IRender.h
    ImageOptions.h
    ImageOptionsInline.h
    LUTOptions.h
    LUTOptionsInline.h
    MemoryReference.h
    Player.h
    PlayerInline.h
    PlayerOptions.h
    PlayerOptionsInline.h
    ReadCache.h
    RenderOptions.h
    RenderOptionsInline.h
    RenderUtil.h
    TimeUnits.h
    Timeline.h
    Transition.h
    Util.h
    UtilInline.h
    Video.h
    VideoInline.h)
set(PRIVATE_HEADERS
    PlayerPrivate.h
    TimelinePrivate.h)

set(SOURCE
    ColorConfigOptions.cpp
    CompareOptions.cpp
    DisplayOptions.cpp
    IRender.cpp
    ImageOptions.cpp
    LUTOptions.cpp
    MemoryReference.cpp
    Player.cpp
    PlayerOptions.cpp
    PlayerPrivate.cpp
    ReadCache.cpp
    RenderUtil.cpp
    TimeUnits.cpp
    Timeline.cpp
    TimelineCreate.cpp
    TimelinePrivate.cpp
    Transition.cpp
    Util.cpp)

add_library(tlTimeline ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
