set(HEADERS
    MeshTest.h)
set(SOURCE
    MeshTest.cpp)

add_library(tlGLTest ${SOURCE} ${HEADERS})
target_link_libraries(tlGLTest tlTestLib tlGL)
set_target_properties(tlGLTest PROPERTIES FOLDER tests)
