set(HEADERS
    App.h
    AudioActions.h
    AudioTool.h
    ColorModel.h
    ColorTool.h
    CompareActions.h
    CompareTool.h
    DevicesModel.h
    DevicesTool.h
    DockTitleBar.h
    FileActions.h
    FilesAModel.h
    FilesBModel.h
    FilesTableModel.h
    FilesTool.h
    FilesView.h
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
    ToolActions.h
    ToolWidget.h
    ViewActions.h
    WindowActions.h)

set(SOURCE
    App.cpp
    AudioActions.cpp
    AudioTool.cpp
    ColorModel.cpp
    ColorTool.cpp
    CompareActions.cpp
    CompareTool.cpp
    DevicesModel.cpp
    DevicesTool.cpp
    DockTitleBar.cpp
    FileActions.cpp
    FilesAModel.cpp
    FilesBModel.cpp
    FilesTableModel.cpp
    FilesTool.cpp
    FilesView.cpp
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
    ToolActions.cpp
    ToolWidget.cpp
    ViewActions.cpp
    WindowActions.cpp)

if(TLRENDER_QT6)
    qt6_add_resources(SOURCE tlQtPlayApp.qrc)
elseif(TLRENDER_QT5)
    qt5_add_resources(SOURCE tlQtPlayApp.qrc)
endif()

add_library(tlQtPlayApp ${HEADERS} ${SOURCE})
target_link_libraries(tlQtPlayApp tlQtWidget tlPlay tlApp)
set_target_properties(tlQtPlayApp PROPERTIES FOLDER lib)
set_target_properties(tlQtPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlQtPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlQtPlayApp)
