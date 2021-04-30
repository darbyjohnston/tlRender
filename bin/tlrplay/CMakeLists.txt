set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    Render.cpp
    Window.cpp
    main.cpp)

add_executable(tlrplay ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay tlrApp GLFW)

install(
    TARGETS tlrplay
    RUNTIME DESTINATION bin)
set_target_properties(tlrplay PROPERTIES FOLDER bin)
