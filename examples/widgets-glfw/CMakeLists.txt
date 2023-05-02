set(HEADERS
    App.h
    BasicWidgets.h
    Charts.h
    GridLayouts.h
    MainWindow.h
    NumericWidgets.h
    RowLayouts.h
    ScrollAreas.h)

set(SOURCE
    App.cpp
    Charts.cpp
    BasicWidgets.cpp
    GridLayouts.cpp
    MainWindow.cpp
    NumericWidgets.cpp
    RowLayouts.cpp
    ScrollAreas.cpp
    main.cpp)

add_executable(widgets-glfw ${SOURCE} ${HEADERS})
target_link_libraries(widgets-glfw tlGLFWApp)
set_target_properties(widgets-glfw PROPERTIES FOLDER examples)
