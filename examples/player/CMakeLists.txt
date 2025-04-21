set(HEADERS
    App.h
    CompareActions.h
    FileActions.h
    FilesModel.h
    MainWindow.h
    MenuBar.h
    PlaybackActions.h
    PlaybackBar.h
    RecentFilesModel.h
    SettingsModel.h
    SettingsWidget.h
    TabBar.h
    ToolBars.h
    ViewActions.h
    WindowActions.h)
set(SOURCE
    App.cpp
    CompareActions.cpp
    FileActions.cpp
    FilesModel.cpp
    MainWindow.cpp
    MenuBar.cpp
    PlaybackActions.cpp
    PlaybackBar.cpp
    RecentFilesModel.cpp
    SettingsModel.cpp
    SettingsWidget.cpp
    TabBar.cpp
    ToolBars.cpp
    ViewActions.cpp
    WindowActions.cpp
    main.cpp)

add_executable(player ${HEADERS} ${SOURCE})
target_link_libraries(player tlTimelineUI)
set_target_properties(player PROPERTIES FOLDER examples)
