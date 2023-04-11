set(HEADERS
    App.h
    MainWindow.h)

set(SOURCE
    App.cpp
    MainWindow.cpp
    main.cpp)

add_executable(timeline-qtwidget ${SOURCE} ${HEADERS})
target_link_libraries(timeline-qtwidget tlQtWidget tlApp)
set_target_properties(timeline-qtwidget PROPERTIES FOLDER examples)
