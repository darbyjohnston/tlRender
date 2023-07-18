include(ExternalProject)

ExternalProject_Add(
    NASM
    URL https://github.com/netwide-assembler/nasm/archive/refs/tags/nasm-2.16.01.tar.gz
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1)

