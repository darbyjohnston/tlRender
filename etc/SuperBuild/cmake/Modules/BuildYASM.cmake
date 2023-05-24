include( ExternalProject )

# Use this for cuting edge yasm
# GIT_REPOSITORY "https://github.com/yasm/yasm.git"

ExternalProject_Add(
    YASM
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/YASM
    URL "http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz"
    CONFIGURE_COMMAND ./configure --prefix=${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
)
