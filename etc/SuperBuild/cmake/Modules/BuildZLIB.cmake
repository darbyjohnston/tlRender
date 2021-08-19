include(ExternalProject)

set(ZLIB_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DSKIP_INSTALL_FILES=ON)

ExternalProject_Add(
    ZLIB
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ZLIB
    URL http://www.zlib.net/zlib-1.2.11.tar.gz
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/ZLIB-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${ZLIB_ARGS})
