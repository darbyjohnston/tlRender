set(HEADERS
    App.h
    MainWindow.h
    SettingsObject.h
    SettingsWidget.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    SettingsObject.cpp
    SettingsWidget.cpp
    main.cpp)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(tlrplay-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay-qwidget tlrQt tlrApp)
set_target_properties(tlrplay-qwidget PROPERTIES FOLDER bin)

install(
    TARGETS tlrplay-qwidget
    RUNTIME DESTINATION bin)
