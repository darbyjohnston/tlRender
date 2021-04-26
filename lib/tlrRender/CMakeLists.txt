set(HEADERS
    FontSystem.h
    Mesh.h
    MeshInline.h
    OffscreenBuffer.h
    OffscreenBufferInline.h
    Render.h
    Shader.h
    ShaderInline.h
    Texture.h
    TextureInline.h)
set(SOURCE
    FontSystem.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    Shader.cpp
    Texture.cpp)

add_library(tlrRender ${HEADERS} ${SOURCE})
target_link_libraries(tlrRender tlrAV glad OpenGL::GL FreeType)
set_target_properties(tlrRender PROPERTIES FOLDER lib)
