include(ExternalProject)

ExternalProject_Add(
    NASM
    URL http://www.nasm.us/pub/nasm/releasebuilds/2.15.05/nasm-2.15.05.tar.xz
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1)

