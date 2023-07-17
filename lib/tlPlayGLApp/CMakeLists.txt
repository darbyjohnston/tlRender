set(HEADERS
    App.h
    AudioMenu.h
    AudioToolWidget.h
    ColorToolWidget.h
    CompareMenu.h
    CompareToolBar.h
    CompareToolWidget.h
    DevicesToolWidget.h
    FileMenu.h
    FileToolBar.h
    FilesToolWidget.h
    FrameMenu.h
    IToolWidget.h
    InfoToolWidget.h
    MainWindow.h
    MessagesToolWidget.h
    PlaybackMenu.h
    RenderMenu.h
    SettingsToolWidget.h
    SystemLogToolWidget.h
    Tools.h
    ToolsMenu.h
    ToolsToolBar.h
    ToolsWidget.h
    ViewMenu.h
    ViewToolBar.h
    WindowMenu.h
    WindowToolBar.h)

set(SOURCE
    App.cpp
    AudioMenu.cpp
    AudioToolWidget.cpp
    ColorToolWidget.cpp
    CompareMenu.cpp
    CompareToolBar.cpp
    CompareToolWidget.cpp
    DevicesToolWidget.cpp
    FileMenu.cpp
    FileToolBar.cpp
    FilesToolWidget.cpp
    FrameMenu.cpp
    IToolWidget.cpp
    InfoToolWidget.cpp
    MainWindow.cpp
    MessagesToolWidget.cpp
    PlaybackMenu.cpp
    RenderMenu.cpp
    SettingsToolWidget.cpp
    SystemLogToolWidget.cpp
    ViewMenu.cpp
    ViewToolBar.cpp
    Tools.cpp
    ToolsMenu.cpp
    ToolsToolBar.cpp
    ToolsWidget.cpp
    WindowMenu.cpp
    WindowToolBar.cpp)

add_library(tlPlayGLApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayGLApp tlPlay tlTimelineUI tlGLApp)
set_target_properties(tlPlayGLApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayGLApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayGLApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayGLApp)
