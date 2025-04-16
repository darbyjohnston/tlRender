set(HEADERS
    App.h
    FileActions.h
    MainWindow.h
    MenuBar.h
    PlaybackActions.h
    PlaybackBar.h
    ToolBar.h
    ViewActions.h
    WindowActions.h)
set(SOURCE
    App.cpp
    FileActions.cpp
    MainWindow.cpp
    MenuBar.cpp
    PlaybackActions.cpp
    PlaybackBar.cpp
    ToolBar.cpp
    ViewActions.cpp
    WindowActions.cpp
    main.cpp)

add_executable(player ${HEADERS} ${SOURCE})
target_link_libraries(player tlTimelineUI)
set_target_properties(player PROPERTIES FOLDER examples)
