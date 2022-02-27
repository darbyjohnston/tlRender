set(HEADERS
    Mesh.h
    OffscreenBuffer.h
    Render.h
    Shader.h
    Texture.h
    Util.h)
set(SOURCE
    Mesh.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    Shader.cpp
    Texture.cpp
    Util.cpp)

add_library(tlRenderGL ${HEADERS} ${SOURCE})
target_link_libraries(tlRenderGL tlTimeline tlGlad)
set_target_properties(tlRenderGL PROPERTIES FOLDER lib)
set_target_properties(tlRenderGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlRenderGL
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlRenderGL)
