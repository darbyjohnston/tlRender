include(ExternalProject)

set(OpenSSL_DEPENDS)
if(NOT WIN32)
    list(APPEND OpenSSL_DEPENDS NASM)
endif()
list(APPEND OpenSSL_DEPENDS ZLIB)

set(OpenSSL_GIT_REPOSITORY "https://github.com/openssl/openssl.git")
set(OpenSSL_GIT_TAG "openssl-3.3.0")

if(WIN32)
    set(OpenSSL_CONFIGURE
        perl Configure VC-WIN64A
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-docs
        no-external-tests
        no-tests
        no-unit-test)
    set(OpenSSL_BUILD nmake)
    set(OpenSSL_INSTALL nmake install)
elseif(APPLE)
    set(OpenSSL_CONFIGURE
        ./Configure
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-docs
        no-external-tests
        no-tests
        no-unit-test)
    if(CMAKE_OSX_DEPLOYMENT_TARGET)
        list(APPEND OpenSSL_CONFIGURE -mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET})
    endif()
    set(OpenSSL_BUILD make)
    set(OpenSSL_INSTALL make install)
else()
    set(OpenSSL_CONFIGURE
        ./Configure
        --prefix=${CMAKE_INSTALL_PREFIX}
        --openssldir=${CMAKE_INSTALL_PREFIX}
        no-docs
        no-external-tests
        no-tests
        no-unit-test)
    set(OpenSSL_BUILD make)
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
