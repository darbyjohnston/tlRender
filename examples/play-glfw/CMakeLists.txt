set(HEADERS
    App.h
    AudioMenu.h
    CompareMenu.h
    FileMenu.h
    FileToolBar.h
    FrameMenu.h
    MainWindow.h
    PlaybackMenu.h
    RenderMenu.h
    ToolsMenu.h
    ViewMenu.h
    WindowMenu.h)

set(SOURCE
    App.cpp
    AudioMenu.cpp
    CompareMenu.cpp
    FileMenu.cpp
    FileToolBar.cpp
    FrameMenu.cpp
    MainWindow.cpp
    PlaybackMenu.cpp
    RenderMenu.cpp
    ViewMenu.cpp
    ToolsMenu.cpp
    WindowMenu.cpp
    main.cpp)

add_executable(play-glfw ${SOURCE} ${HEADERS})
target_link_libraries(play-glfw tlGLFWApp tlTimelineUI)
set_target_properties(play-glfw PROPERTIES FOLDER examples)
