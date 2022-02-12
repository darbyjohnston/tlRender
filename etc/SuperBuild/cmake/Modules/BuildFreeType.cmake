include(ExternalProject)

set(FreeType_GIT_REPOSITORY "https://github.com/freetype/freetype.git")
# \bug Version 2-11-0 crashes on Windows with "writing_system_class was nullptr"
set(FreeType_GIT_TAG "6a2b3e4007e794bfc6c91030d0ed987f925164a8") # tag: VER-2-10-4

set(FreeType_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DFT_WITH_ZLIB=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE)

ExternalProject_Add(
    FreeType
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FreeType
    DEPENDS ZLIB
    GIT_REPOSITORY ${FreeType_GIT_REPOSITORY}
    GIT_TAG ${FreeType_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${FreeType_ARGS})
