set(HEADERS
    App.h)

set(SOURCE
    App.cpp
    main.cpp)

add_executable(ui-glfw ${SOURCE} ${HEADERS})
target_link_libraries(ui-glfw tlApp tlUI tlGL GLFW)
set_target_properties(ui-glfw PROPERTIES FOLDER examples)
