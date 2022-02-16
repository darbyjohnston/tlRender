set(HEADERS
    App.h
	AudioTool.h
	ColorModel.h
    ColorTool.h
	CompareTool.h
	FilesModel.h
	FilesTool.h
	FilesView.h
    ImageTool.h
    InfoTool.h
	InfoModel.h
    MainWindow.h
	MessagesTool.h
	OpenWithAudioDialog.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsTool.h
	SystemLogTool.h
	ToolWidget.h)
set(SOURCE
    App.cpp
	AudioTool.cpp
	ColorModel.cpp
    ColorTool.cpp
	CompareTool.cpp
	FilesModel.cpp
	FilesTool.cpp
	FilesView.cpp
    ImageTool.cpp
    InfoTool.cpp
	InfoModel.cpp
    MainWindow.cpp
	MessagesTool.cpp
	OpenWithAudioDialog.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
	SystemLogTool.cpp
	ToolWidget.cpp)
if(TLRENDER_BUILD_QT6)
	qt6_add_resources(SOURCE tlPlay.qrc)
elseif(TLRENDER_BUILD_QT5)
	qt5_add_resources(SOURCE tlPlay.qrc)
endif()

add_library(tlPlay ${HEADERS} ${SOURCE})
if(TLRENDER_BUILD_QT6)
	target_link_libraries(tlPlay tlQWidget tlApp)
elseif(TLRENDER_BUILD_QT5)
	target_link_libraries(tlPlay tlQWidget tlApp)
endif()
set_target_properties(tlPlay PROPERTIES FOLDER lib)
set_target_properties(tlPlay PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlPlay
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlPlay)
