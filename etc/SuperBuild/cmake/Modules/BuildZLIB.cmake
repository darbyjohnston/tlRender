include(ExternalProject)

set(ZLIB_GIT_REPOSITORY "https://github.com/madler/zlib.git")
set(ZLIB_GIT_TAG "cacf7f1d4e3d44d871b605da3b647f07d718623f") # tag: "v1.2.11"

set(ZLIB_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DSKIP_INSTALL_FILES=ON)

ExternalProject_Add(
    ZLIB
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ZLIB
    GIT_REPOSITORY ${ZLIB_GIT_REPOSITORY}
    GIT_TAG ${ZLIB_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ZLIB-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${ZLIB_ARGS})
