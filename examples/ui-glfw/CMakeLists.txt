set(HEADERS
    App.h
    BasicWidget.h
    GridLayoutWidget.h
    MainWindow.h
    NumericWidget.h
    RowLayoutWidget.h)

set(SOURCE
    App.cpp
    BasicWidget.cpp
    GridLayoutWidget.cpp
    MainWindow.cpp
    NumericWidget.cpp
    RowLayoutWidget.cpp
    main.cpp)

add_executable(ui-glfw ${SOURCE} ${HEADERS})
target_link_libraries(ui-glfw tlGLFWApp)
set_target_properties(ui-glfw PROPERTIES FOLDER examples)
