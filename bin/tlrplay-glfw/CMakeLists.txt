set(HEADERS
    App.h
    FontSystem.h
    Mesh.h
    OffscreenBuffer.h
    Render.h
    Shader.h
    Texture.h
    Util.h)
set(SOURCE
    App.cpp
    FontSystem.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    Shader.cpp
    Texture.cpp
    Util.cpp
    main.cpp)

add_executable(tlrplay-glfw ${SOURCE} ${HEADERS})
target_link_libraries(tlrplay-glfw tlrApp GLFW glad OpenGL::GL FreeType)

install(
    TARGETS tlrplay-glfw
    RUNTIME DESTINATION bin)
set_target_properties(tlrplay-glfw PROPERTIES FOLDER bin)
