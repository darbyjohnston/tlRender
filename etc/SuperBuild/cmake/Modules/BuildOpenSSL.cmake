include(ExternalProject)

set(OpenSSL_DEPENDS)
if(NOT WIN32)
    list(APPEND OpenSSL_DEPENDS NASM)
endif()
list(APPEND OpenSSL_DEPENDS ZLIB)

set(OpenSSL_GIT_REPOSITORY "https://github.com/openssl/openssl.git")
set(OpenSSL_GIT_TAG "openssl-3.1.4")

if(WIN32)
    set(OpenSSL_CONFIGURE
        perl Configure VC-WIN64A
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-external-tests
        no-tests
        no-unit-test)
    set(OpenSSL_BUILD nmake install)
    set(OpenSSL_INSTALL nmake install)
else()
    set(OpenSSL_CONFIGURE
        ./Configure
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-external-tests
        no-tests
        no-unit-test)
    set(OpenSSL_BUILD make install)
    set(OpenSSL_INSTALL make install)
endif()

ExternalProject_Add(
    OpenSSL
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/OpenSSL
    DEPENDS ${OpenSSL_DEPENDS}
    GIT_REPOSITORY ${OpenSSL_GIT_REPOSITORY}
    GIT_TAG ${OpenSSL_GIT_TAG}
    CONFIGURE_COMMAND ${OpenSSL_CONFIGURE}
    BUILD_COMMAND ${OpenSSL_BUILD}
    INSTALL_COMMAND ${OpenSSL_INSTALL}
    BUILD_IN_SOURCE 1)
