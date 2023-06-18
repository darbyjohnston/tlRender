set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

add_executable(render-glfw ${SOURCE} ${HEADERS})
target_link_libraries(render-glfw tlApp)
set_target_properties(render-glfw PROPERTIES FOLDER examples)
