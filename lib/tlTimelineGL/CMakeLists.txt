set(HEADERS
    Render.h)
set(PRIVATE_HEADERS
    RenderPrivate.h)

set(SOURCE
    Render.cpp
    RenderPrims.cpp
    RenderVideo.cpp)
if("${ftk_API}" STREQUAL "GL_4_1" OR "${ftk_API}" STREQUAL "GL_4_1_Debug")
    list(APPEND SOURCE RenderShaders_GL_4_1.cpp)
elseif("${ftk_API}" STREQUAL "GLES_2")
    list(APPEND SOURCE RenderShaders_GLES_2.cpp)
endif()

add_library(tlTimelineGL ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlTimelineGL tlTimeline)
set_target_properties(tlTimelineGL PROPERTIES FOLDER lib)
set_target_properties(tlTimelineGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlTimelineGL
    EXPORT tlTimelineGLTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlTimelineGL)
install(
    EXPORT tlTimelineGLTargets
    FILE tlTimelineGLTargets.cmake
    DESTINATION "lib/cmake/tlRender"
    NAMESPACE tlRender::)
