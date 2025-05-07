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
    StatusBar.h
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
    StatusBar.cpp
    TabBar.cpp
    ToolBars.cpp
    ViewActions.cpp
    WindowActions.cpp
    main.cpp)

add_executable(tlplay ${HEADERS} ${SOURCE})
target_link_libraries(tlplay tlTimelineUI)
set_target_properties(tlplay PROPERTIES FOLDER bin)

install(
    TARGETS tlplay
    RUNTIME DESTINATION bin)
