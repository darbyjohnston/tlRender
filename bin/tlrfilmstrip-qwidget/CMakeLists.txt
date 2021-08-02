set(HEADERS
    App.h
    MainWindow.h
    SettingsObject.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    SettingsObject.cpp
    main.cpp)

add_executable(tlrfilmstrip-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(tlrfilmstrip-qwidget tlrQt tlrApp)
set_target_properties(tlrfilmstrip-qwidget PROPERTIES FOLDER bin)

install(
    TARGETS tlrfilmstrip-qwidget
    RUNTIME DESTINATION bin)
