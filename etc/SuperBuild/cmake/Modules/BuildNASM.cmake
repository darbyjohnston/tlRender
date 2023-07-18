include(ExternalProject)

ExternalProject_Add(
    NASM
    https://github.com/netwide-assembler/nasm/archive/refs/tags/nasm-2.15.05.tar.gz
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/NASM-patch/configure
        ${CMAKE_CURRENT_BINARY_DIR}/NASM/src/NASM/configure
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1)
