set(HEADERS
    App.h
	AudioTool.h
	FilesModel.h
	FilesTool.h
	FilesView.h
    ImageTool.h
    MainWindow.h
	OpenWithAudioDialog.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsTool.h
	ToolWidget.h)
set(SOURCE
    App.cpp
	AudioTool.cpp
	FilesModel.cpp
	FilesTool.cpp
	FilesView.cpp
    ImageTool.cpp
    MainWindow.cpp
	OpenWithAudioDialog.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
	ToolWidget.cpp
    main.cpp)

add_executable(play-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(play-qwidget tlrQWidget tlrApp)
set_target_properties(play-qwidget PROPERTIES FOLDER examples)
