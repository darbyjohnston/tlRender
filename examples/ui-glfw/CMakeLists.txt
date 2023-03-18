set(HEADERS
    App.h
    ButtonWidget.h
    MainWindow.h
    RowLayoutWidget.h)

set(SOURCE
    App.cpp
    ButtonWidget.cpp
    MainWindow.cpp
    RowLayoutWidget.cpp
    main.cpp)

add_executable(ui-glfw ${SOURCE} ${HEADERS})
target_link_libraries(ui-glfw tlApp tlUI tlGL GLFW)
set_target_properties(ui-glfw PROPERTIES FOLDER examples)
