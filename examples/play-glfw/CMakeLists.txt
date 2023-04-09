set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlApp tlGL GLFW)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
