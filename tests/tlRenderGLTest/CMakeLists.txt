set(HEADERS
    MeshTest.h)
set(SOURCE
    MeshTest.cpp)

add_library(tlRenderGLTest ${SOURCE} ${HEADERS})
target_link_libraries(tlRenderGLTest tlTestLib tlRenderGL)
set_target_properties(tlRenderGLTest PROPERTIES FOLDER tests)
