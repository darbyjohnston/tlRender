include(ExternalProject)

set(TIFF_GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff.git")
set(TIFF_GIT_TAG "d21dcc67d0d3f4686ee989a085ad2bea9c58259d") # tag: v4.3.0

set(TIFF_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -Dold-jpeg=OFF
    -Djbig=OFF
    -Dlzma=OFF
    -Dzstd=OFF
    -Dwebp=OFF
    -Djpeg12=OFF
    -Dlibdeflate=OFF)

ExternalProject_Add(
    TIFF
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/TIFF
    DEPENDS ZLIB JPEG
    GIT_REPOSITORY ${TIFF_GIT_REPOSITORY}
    GIT_TAG ${TIFF_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/TIFF-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/TIFF/src/TIFF/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${TIFF_ARGS})
