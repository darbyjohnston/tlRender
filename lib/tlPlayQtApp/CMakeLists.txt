set(HEADERS
    App.h
    AudioActions.h
    AudioTool.h
    ColorConfigModel.h
    ColorTool.h
    CompareActions.h
    CompareTool.h
    DevicesModel.h
    DevicesTool.h
    DockTitleBar.h
    FileActions.h
    FilesAModel.h
    FilesBModel.h
    FilesTool.h
    FilesView.h
    FrameActions.h
    IFilesTableModel.h
    IToolWidget.h
    InfoTool.h
    InfoModel.h
    MainWindow.h
    MemoryTimeline.h
    MessagesTool.h
    OpenSeparateAudioDialog.h
    PlaybackActions.h
    RenderActions.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsTool.h
    SystemLogTool.h
    TimelineActions.h
    ToolActions.h
    ViewActions.h
    WindowActions.h)

set(SOURCE
    App.cpp
    AudioActions.cpp
    AudioTool.cpp
    ColorConfigModel.cpp
    ColorTool.cpp
    CompareActions.cpp
    CompareTool.cpp
    DevicesModel.cpp
    DevicesTool.cpp
    DockTitleBar.cpp
    FileActions.cpp
    FilesAModel.cpp
    FilesBModel.cpp
    FilesTool.cpp
    FilesView.cpp
    FrameActions.cpp
    IFilesTableModel.cpp
    IToolWidget.cpp
    InfoTool.cpp
    InfoModel.cpp
    MainWindow.cpp
    MemoryTimeline.cpp
    MessagesTool.cpp
    OpenSeparateAudioDialog.cpp
    PlaybackActions.cpp
    RenderActions.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
    SystemLogTool.cpp
    TimelineActions.cpp
    ToolActions.cpp
    ViewActions.cpp
    WindowActions.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE tlPlayQtApp.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE tlPlayQtApp.qrc)
endif()

add_library(tlPlayQtApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayQtApp tlQtWidget tlPlay tlApp)
set_target_properties(tlPlayQtApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayQtApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayQtApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayQtApp)
