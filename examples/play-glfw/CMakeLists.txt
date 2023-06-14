set(HEADERS
    App.h
    AudioMenu.h
    CompareMenu.h
    FileMenu.h
    MainWindow.h
    PlaybackMenu.h
    RenderMenu.h
    ViewMenu.h
    WindowMenu.h)

set(SOURCE
    App.cpp
    AudioMenu.cpp
    CompareMenu.cpp
    FileMenu.cpp
    MainWindow.cpp
    PlaybackMenu.cpp
    RenderMenu.cpp
    ViewMenu.cpp
    WindowMenu.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlGLFWApp tlTimelineUI)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
