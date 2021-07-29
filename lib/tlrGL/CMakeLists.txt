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
target_link_libraries(tlrGL tlrCore glad OpenGL::GL OCIO FreeType)
set_target_properties(tlrGL PROPERTIES FOLDER lib)
set_target_properties(tlrGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlrGL
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlrGL)
