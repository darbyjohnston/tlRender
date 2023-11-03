include(ExternalProject)

set(CURL_DEPENDS Libssh2 ZLIB)
if(NOT WIN32)
    list(APPEND CURL_DEPENDS OpenSSL)
endif()

set(CURL_GIT_REPOSITORY "https://github.com/curl/curl.git")
set(CURL_GIT_TAG "curl-8_4_0")

set(CURL_ARGS
    -DBUILD_CURL_EXE=OFF
    -DCURL_USE_OPENSSL=ON
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    CURL
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/CURL
    DEPENDS ${CURL_DEPENDS}
    GIT_REPOSITORY ${CURL_GIT_REPOSITORY}
    GIT_TAG ${CURL_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${CURL_ARGS})
