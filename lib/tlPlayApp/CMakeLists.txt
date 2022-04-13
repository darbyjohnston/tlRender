set(HEADERS
    App.h
	AudioActions.h
	AudioTool.h
	ColorModel.h
    ColorTool.h
	CompareActions.h
	CompareTool.h
	FileActions.h
	FilesModel.h
	FilesTool.h
	FilesView.h
	ImageActions.h
    InfoTool.h
	InfoModel.h
    MainWindow.h
	MessagesTool.h
	OpenWithAudioDialog.h
	OutputTool.h
	PlaybackActions.h
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
	FileActions.cpp
	FilesModel.cpp
	FilesTool.cpp
	FilesView.cpp
	ImageActions.cpp
    InfoTool.cpp
	InfoModel.cpp
    MainWindow.cpp
	MessagesTool.cpp
	OpenWithAudioDialog.cpp
	OutputTool.cpp
	PlaybackActions.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
	SystemLogTool.cpp
	ToolWidget.cpp
	ViewActions.cpp
	WindowActions.cpp)

if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlPlayApp.qrc)
elseif(TLRENDER_BUILD_QT5)
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
