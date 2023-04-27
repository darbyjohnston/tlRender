set(HEADERS
    Mesh.h
    OffscreenBuffer.h
    Render.h
    Shader.h
    Texture.h
    Util.h)
set(PRIVATE_HEADERS
    RenderPrivate.h)

set(SOURCE
    Mesh.cpp
    Mesh.cpp
    OffscreenBuffer.cpp
    Render.cpp
    RenderPrims.cpp
    RenderShaders.cpp
    RenderVideo.cpp
    Shader.cpp
    Texture.cpp
    Util.cpp)

set(LIBRARIES tlTimeline)
set(LIBRARIES_PRIVATE tlGlad)

add_library(tlGL ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlGL ${LIBRARIES} ${LIBRARIES_PRIVATE})
set_target_properties(tlGL PROPERTIES FOLDER lib)
set_target_properties(tlGL PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGL
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGL)
