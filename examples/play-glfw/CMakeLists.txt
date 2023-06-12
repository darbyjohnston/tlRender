set(HEADERS
    App.h
    FileMenu.h
    MainWindow.h)

set(SOURCE
    App.cpp
    FileMenu.cpp
    MainWindow.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlGLFWApp tlTimelineUI)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
