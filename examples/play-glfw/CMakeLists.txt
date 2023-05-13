set(HEADERS
    App.h
    MainWindow.h)

set(SOURCE
    App.cpp
    MainWindow.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlGLFWApp tlTimelineUI)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
