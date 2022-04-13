set(HEADERS
    App.h
    Util.h)

set(SOURCE
    App.cpp
    Util.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlApp tlGL GLFW)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
