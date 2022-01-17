set(HEADERS
    App.h
	AudioTool.h
	CompareTool.h
	FilesModel.h
	FilesTool.h
    ImageTool.h
	LayersModel.h
	LayersTool.h
    MainWindow.h
	OpenWithAudioDialog.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsTool.h)
set(SOURCE
    App.cpp
	AudioTool.cpp
	CompareTool.cpp
	FilesModel.cpp
	FilesTool.cpp
    ImageTool.cpp
	LayersModel.cpp
	LayersTool.cpp
    MainWindow.cpp
	OpenWithAudioDialog.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsTool.cpp
    main.cpp)

add_executable(play-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(play-qwidget tlrQWidget tlrApp)
set_target_properties(play-qwidget PROPERTIES FOLDER examples)
