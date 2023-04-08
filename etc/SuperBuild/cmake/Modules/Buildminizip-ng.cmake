include(ExternalProject)

set(minizip-ng_GIT_REPOSITORY "https://github.com/zlib-ng/minizip-ng.git")
set(minizip-ng_GIT_TAG "3.0.9")

set(minizip-ng_ARGS
    -DMZ_FETCH_LIBS=OFF
    -DMZ_LZMA=OFF
    -DMZ_ZSTD=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    minizip-ng
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/minizip-ng
    GIT_REPOSITORY ${minizip-ng_GIT_REPOSITORY}
    GIT_TAG ${minizip-ng_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${minizip-ng_ARGS})
