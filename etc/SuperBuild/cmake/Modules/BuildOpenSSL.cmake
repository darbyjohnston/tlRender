include(ExternalProject)

set(OpenSSL_GIT_REPOSITORY "https://github.com/openssl/openssl.git")
set(OpenSSL_GIT_TAG "openssl-3.1.4")

set(OpenSSL_CONFIGURE_ARGS
    --prefix=${CMAKE_INSTALL_PREFIX}
    --openssldir=${CMAKE_INSTALL_PREFIX}
    no-shared
    no-zlib)

ExternalProject_Add(
    OpenSSL
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenSSL
    DEPENDS NASM
    GIT_REPOSITORY ${OpenSSL_GIT_REPOSITORY}
    GIT_TAG ${OpenSSL_GIT_TAG}
    CONFIGURE_COMMAND ./Configure ${OpenSSL_CONFIGURE_ARGS}
    INSTALL_COMMAND make install
    BUILD_IN_SOURCE 1)
