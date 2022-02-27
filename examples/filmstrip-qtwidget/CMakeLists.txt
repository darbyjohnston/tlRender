set(HEADERS
    App.h
    MainWindow.h)
set(SOURCE
    App.cpp
    MainWindow.cpp
    main.cpp)

add_executable(filmstrip-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(filmstrip-qtwidget tlQtWidget tlApp)
set_target_properties(filmstrip-qtwidget PROPERTIES FOLDER examples)
