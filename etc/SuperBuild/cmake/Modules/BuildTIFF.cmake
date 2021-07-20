include(ExternalProject)

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
    URL http://download.osgeo.org/libtiff/tiff-4.1.0.tar.gz
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/TIFF-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/TIFF/src/TIFF/CMakeLists.txt
    CMAKE_ARGS ${TIFF_ARGS})

