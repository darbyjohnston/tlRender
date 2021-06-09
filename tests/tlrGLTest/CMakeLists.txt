set(HEADERS
    MeshTest.h)
set(SOURCE
    MeshTest.cpp)

add_library(tlrGLTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrGLTest tlrTestLib tlrGL)
set_target_properties(tlrGLTest PROPERTIES FOLDER tests)
