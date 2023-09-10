include(ExternalProject)

set(glfw3_GIT_REPOSITORY "https://github.com/glfw/glfw.git")
set(glfw3_GIT_TAG "3.3.8")

set(glfw3_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DGLFW_SHARED_LIBS=${BUILD_SHARED_LIBS}
    -DGLFW_BUILD_EXAMPLES=FALSE
    -DGLFW_BUILD_TESTS=FALSE
    -DGLFW_BUILD_DOCS=FALSE)

ExternalProject_Add(
    glfw3
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/glfw3
    GIT_REPOSITORY ${glfw3_GIT_REPOSITORY}
    GIT_TAG ${glfw3_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${glfw3_ARGS})

