include(ExternalProject)

set(TIFF_GIT_REPOSITORY "https://gitlab.com/libtiff/libtiff.git")
set(TIFF_GIT_TAG "e0d707dc1524d8c0e20f03396f234e0f1b07b3f4") # tag: v4.1.0

set(TIFF_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -Dold-jpeg=OFF
    -Djbig=OFF
    -Dlzma=OFF
    -Dzstd=OFF
    -Dwebp=OFF
    -Djpeg12=OFF)

ExternalProject_Add(
    TIFF
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/TIFF
    DEPENDS ZLIB JPEG
    GIT_REPOSITORY ${TIFF_GIT_REPOSITORY}
    GIT_TAG ${TIFF_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/TIFF-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/TIFF/src/TIFF/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${TIFF_ARGS})
