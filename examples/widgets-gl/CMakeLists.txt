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

add_executable(widgets-gl ${SOURCE} ${HEADERS})
target_link_libraries(widgets-gl tlGLApp)
set_target_properties(widgets-gl PROPERTIES FOLDER examples)
