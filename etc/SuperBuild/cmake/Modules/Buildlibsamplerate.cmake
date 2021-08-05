include(ExternalProject)

set(libsamplerate_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DLIBSAMPLERATE_EXAMPLES=OFF)

ExternalProject_Add(
    libsamplerate
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsamplerate
    GIT_REPOSITORY https://github.com/libsndfile/libsamplerate.git
    GIT_TAG 0.2.1
    LIST_SEPARATOR |
    CMAKE_ARGS ${libsamplerate_ARGS})
