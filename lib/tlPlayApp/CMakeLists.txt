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
	FilesModel.h
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
	FilesModel.cpp
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
	ToolWidget.cpp
	ViewActions.cpp
	WindowActions.cpp)

if(TLRENDER_QT6)
	qt6_add_resources(SOURCE tlPlayApp.qrc)
elseif(TLRENDER_QT5)
	qt5_add_resources(SOURCE tlPlayApp.qrc)
endif()

add_library(tlPlayApp ${HEADERS} ${SOURCE})
target_link_libraries(tlPlayApp tlQtWidget tlApp)
set_target_properties(tlPlayApp PROPERTIES FOLDER lib)
set_target_properties(tlPlayApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlayApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlPlayApp)
