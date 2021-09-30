include(ExternalProject)

set(libsamplerate_GIT_REPOSITORY "https://github.com/libsndfile/libsamplerate.git")
set(libsamplerate_GIT_TAG "a147b6dc4046deda22b2aa772b348717de772e2b") # tag: 0.2.1

set(libsamplerate_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DLIBSAMPLERATE_EXAMPLES=OFF)

ExternalProject_Add(
    libsamplerate
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsamplerate
    GIT_REPOSITORY ${libsamplerate_GIT_REPOSITORY}
    GIT_TAG ${libsamplerate_GIT_TAG}
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/libsamplerate-patch/CMakeLists.txt
        ${CMAKE_CURRENT_BINARY_DIR}/libsamplerate/src/libsamplerate/CMakeLists.txt
    LIST_SEPARATOR |
    CMAKE_ARGS ${libsamplerate_ARGS})
