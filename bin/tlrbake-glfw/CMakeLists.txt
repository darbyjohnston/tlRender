set(HEADERS
    App.h)
set(SOURCE
    App.cpp
    main.cpp)

add_executable(tlrbake-glfw ${SOURCE} ${HEADERS})
target_link_libraries(tlrbake-glfw tlrGL tlrApp GLFW)

install(
    TARGETS tlrbake-glfw
    RUNTIME DESTINATION bin)
set_target_properties(tlrbake-glfw PROPERTIES FOLDER bin)
