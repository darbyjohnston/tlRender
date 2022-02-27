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
	PlaybackActions.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
	SystemLogTool.cpp
	ToolWidget.cpp
	ViewActions.cpp
	WindowActions.cpp)
if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlAppPlay.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE tlAppPlay.qrc)
endif()

add_library(tlAppPlay ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlAppPlay tlQtWidget tlApp)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlAppPlay tlQtWidget tlApp)
endif()
set_target_properties(tlAppPlay PROPERTIES FOLDER lib)
set_target_properties(tlAppPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlAppPlay
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlAppPlay)
