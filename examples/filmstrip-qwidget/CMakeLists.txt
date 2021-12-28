set(HEADERS
    App.h
    MainWindow.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    main.cpp)

add_executable(filmstrip-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(filmstrip-qwidget tlrQWidget tlrApp)
set_target_properties(filmstrip-qwidget PROPERTIES FOLDER examples)
