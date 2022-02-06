set(HEADERS
    App.h
	AudioTool.h
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
	ToolWidget.cpp
    main.cpp)

add_executable(play-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(play-qwidget tlrQWidget tlrApp)
set_target_properties(play-qwidget PROPERTIES FOLDER examples)
