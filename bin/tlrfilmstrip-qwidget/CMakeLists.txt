set(HEADERS
    App.h
    MainWindow.h
    SettingsObject.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    SettingsObject.cpp
    main.cpp)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(tlrfilmstrip-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(tlrfilmstrip-qwidget tlrQt tlrApp)

install(
    TARGETS tlrfilmstrip-qwidget
    RUNTIME DESTINATION bin)
set_target_properties(tlrfilmstrip-qwidget PROPERTIES FOLDER bin)
