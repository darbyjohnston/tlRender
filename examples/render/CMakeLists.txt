set(HEADERS
    render.h)

set(SOURCE
    render.cpp
    main.cpp)

add_executable(render ${SOURCE} ${HEADERS})
target_link_libraries(render tlTimelineGL)
set_target_properties(render PROPERTIES FOLDER examples)
