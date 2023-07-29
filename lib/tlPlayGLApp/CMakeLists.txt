set(HEADERS
    App.h
    AudioMenu.h
    AudioPopup.h
    AudioTool.h
    ColorTool.h
    CompareMenu.h
    CompareToolBar.h
    DevicesTool.h
    FileMenu.h
    FileToolBar.h
    FilesTool.h
    FrameMenu.h
    IToolWidget.h
    InfoTool.h
    MainWindow.h
    MessagesTool.h
    PlaybackMenu.h
    RenderMenu.h
    SeparateAudioDialog.h
    SeparateAudioPrivate.h
    SpeedPopup.h
    Settings.h
    SettingsInline.h
    SettingsTool.h
    Style.h
    SystemLogTool.h
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
    AudioPopup.cpp
    AudioTool.cpp
    ColorTool.cpp
    CompareMenu.cpp
    CompareToolBar.cpp
    DevicesTool.cpp
    FileMenu.cpp
    FileToolBar.cpp
    FilesTool.cpp
    FrameMenu.cpp
    IToolWidget.cpp
    InfoTool.cpp
    MainWindow.cpp
    MessagesTool.cpp
    PlaybackMenu.cpp
    RenderMenu.cpp
    SeparateAudioDialog.cpp
    SeparateAudioWidget.cpp
    SpeedPopup.cpp
    Settings.cpp
    SettingsTool.cpp
    Style.cpp
    SystemLogTool.cpp
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
