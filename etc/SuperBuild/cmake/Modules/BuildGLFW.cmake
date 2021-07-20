include(ExternalProject)

set(GLFW_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DGLFW_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DGLFW_BUILD_EXAMPLES=FALSE
    -DGLFW_BUILD_TESTS=FALSE
    -DGLFW_BUILD_DOCS=FALSE)

ExternalProject_Add(
    GLFW
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/GLFW
    URL http://github.com/glfw/glfw/releases/download/3.3.3/glfw-3.3.3.zip
    CMAKE_ARGS ${GLFW_ARGS})

