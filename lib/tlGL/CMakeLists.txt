set(HEADERS
    GLFWSystem.h
    Init.h
    Mesh.h
    OffscreenBuffer.h
    Shader.h
    Texture.h
    TextureAtlas.h
    Util.h)
set(PRIVATE_HEADERS)

set(SOURCE
    GLFWSystem.cpp
    Init.cpp
    Mesh.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Shader.cpp
    Texture.cpp
    TextureAtlas.cpp
    Util.cpp)

set(LIBRARIES tlCore GLFW tlGlad)
set(LIBRARIES_PRIVATE)

add_library(tlGL ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlGL ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(tlGL PROPERTIES FOLDER lib)
set_target_properties(tlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGL)
