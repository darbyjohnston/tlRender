set(HEADERS
    FontSystem.h
    Mesh.h
    OffscreenBuffer.h
    Render.h
    Shader.h
    Texture.h
    Util.h)
set(SOURCE
    Mesh.cpp
    FontSystem.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    Shader.cpp
    Texture.cpp
    Util.cpp)

add_library(tlrGL ${HEADERS} ${SOURCE})
target_link_libraries(tlrGL tlrCore glad OpenGL::GL FreeType)
set_target_properties(tlrGL PROPERTIES FOLDER lib)
