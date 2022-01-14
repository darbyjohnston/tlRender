set(HEADERS
    App.h
	AudioSyncWidget.h
    ImageOptionsWidget.h
    MainWindow.h
	OpenWithAudioDialog.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsWidget.h
	TimelineListModel.h)
set(SOURCE
    App.cpp
	AudioSyncWidget.cpp
    ImageOptionsWidget.cpp
    MainWindow.cpp
	OpenWithAudioDialog.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsWidget.cpp
	TimelineListModel.cpp
    main.cpp)

add_executable(play-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(play-qwidget tlrQWidget tlrApp)
set_target_properties(play-qwidget PROPERTIES FOLDER examples)
