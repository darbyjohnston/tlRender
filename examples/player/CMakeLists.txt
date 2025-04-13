set(HEADERS
    App.h
    MainWindow.h
    MenuBar.h
    PlaybackBar.h
    ToolBar.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    MenuBar.cpp
    PlaybackBar.cpp
    ToolBar.cpp
    main.cpp)

add_executable(player ${HEADERS} ${SOURCE})
target_link_libraries(player tlTimelineUI)
set_target_properties(player PROPERTIES FOLDER examples)
