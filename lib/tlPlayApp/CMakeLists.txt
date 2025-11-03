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
    WindowActions.cpp)

set(LIBRARIES tlTimelineUI tlDevice)

add_library(tlPlayApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayApp ${LIBRARIES})
set_target_properties(tlPlayApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayApp)
