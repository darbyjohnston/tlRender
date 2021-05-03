set(HEADERS
    App.h
    GLWidget.h
    MainWindow.h)
set(SOURCE
    App.cpp
    GLWidget.cpp
    MainWindow.cpp
    main.cpp)

#set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_executable(tlrplay-qwidget ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay-qwidget tlrApp Qt5::Widgets OpenGL::GL)

install(
    TARGETS tlrplay-qwidget
    RUNTIME DESTINATION bin)
set_target_properties(tlrplay-qwidget PROPERTIES FOLDER bin)
