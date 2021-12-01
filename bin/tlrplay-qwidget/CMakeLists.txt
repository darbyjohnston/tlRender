set(HEADERS
    App.h
    ImageOptionsWidget.h
    MainWindow.h
	OpenPlusAudioDialog.h
    SecondaryWindow.h
    SettingsObject.h
    SettingsWidget.h)
set(SOURCE
    App.cpp
    ImageOptionsWidget.cpp
    MainWindow.cpp
	OpenPlusAudioDialog.cpp
    SecondaryWindow.cpp
    SettingsObject.cpp
    SettingsWidget.cpp
    main.cpp)

add_executable(tlrplay-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay-qwidget tlrQWidget tlrApp)
set_target_properties(tlrplay-qwidget PROPERTIES FOLDER bin)

install(
    TARGETS tlrplay-qwidget
    RUNTIME DESTINATION bin)
