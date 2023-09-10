include(ExternalProject)

set(freetype_GIT_REPOSITORY "https://github.com/freetype/freetype.git")
set(freetype_GIT_TAG "VER-2-13-0")

set(freetype_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DFT_WITH_ZLIB=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE)

ExternalProject_Add(
    freetype
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/freetype
    DEPENDS ZLIB
    GIT_REPOSITORY ${freetype_GIT_REPOSITORY}
    GIT_TAG ${freetype_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${freetype_ARGS})
