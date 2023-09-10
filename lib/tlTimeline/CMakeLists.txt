set(HEADERS
    Audio.h
    AudioInline.h
    ColorConfigOptions.h
    ColorConfigOptionsInline.h
    CompareOptions.h
    CompareOptionsInline.h
    DisplayOptions.h
    DisplayOptionsInline.h
    Edit.h
    IRender.h
    ImageOptions.h
    ImageOptionsInline.h
    Init.h
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
list(APPEND HEADERS
    GLRender.h)
list(APPEND PRIVATE_HEADERS
    GLRenderPrivate.h)

set(SOURCE
    ColorConfigOptions.cpp
    CompareOptions.cpp
    DisplayOptions.cpp
    Edit.cpp
    IRender.cpp
    ImageOptions.cpp
    Init.cpp
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
list(APPEND SOURCE
    GLRender.cpp
    GLRenderPrims.cpp
    GLRenderVideo.cpp)
if("${TLRENDER_API}" STREQUAL "GL_4_1" OR "${TLRENDER_API}" STREQUAL "GL_4_1_Debug")
    list(APPEND SOURCE GLRenderShaders_GL_4_1.cpp)
elseif("${TLRENDER_API}" STREQUAL "GLES_2")
    list(APPEND SOURCE GLRenderShaders_GLES_2.cpp)
endif()

add_library(tlTimeline ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimeline tlIO)
set_target_properties(tlTimeline PROPERTIES FOLDER lib)
set_target_properties(tlTimeline PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlTimeline
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimeline)
