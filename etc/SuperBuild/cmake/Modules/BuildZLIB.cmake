include(ExternalProject)


set(ZLIB_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DSKIP_INSTALL_FILES=ON)

ExternalProject_Add(
    ZLIB
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ZLIB
    URL "https://zlib.net/fossils/zlib-1.2.11.tar.gz"  # Original repository
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/ZLIB-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/ZLIB/src/ZLIB/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${ZLIB_ARGS})
