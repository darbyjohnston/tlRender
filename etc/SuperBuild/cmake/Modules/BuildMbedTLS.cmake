include(ExternalProject)

set(MbedTLS_GIT_REPOSITORY "https://github.com/Mbed-TLS/mbedtls.git")
set(MbedTLS_GIT_TAG "v3.4.1")

set(MbedTLS_ARGS
    ${TLRENDER_EXTERNAL_ARGS}
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON)

ExternalProject_Add(
    MbedTLS
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/MbedTLS
    GIT_REPOSITORY ${MbedTLS_GIT_REPOSITORY}
    GIT_TAG ${MbedTLS_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${MbedTLS_ARGS})
