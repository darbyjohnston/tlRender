set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

add_executable(render-gl ${SOURCE} ${HEADERS})
target_link_libraries(render-gl tlApp)
set_target_properties(render-gl PROPERTIES FOLDER examples)
