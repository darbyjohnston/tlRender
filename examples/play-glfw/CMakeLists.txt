set(HEADERS
    App.h
    Util.h)
set(SOURCE
    App.cpp
    Util.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlGL tlApp GLFW)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
