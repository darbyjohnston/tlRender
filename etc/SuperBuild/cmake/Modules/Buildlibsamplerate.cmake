include(ExternalProject)

set(libsamplerate_GIT_REPOSITORY "https://github.com/libsndfile/libsamplerate.git")
set(libsamplerate_GIT_TAG "c96f5e3de9c4488f4e6c97f59f5245f22fda22f7") # tag: 0.2.2

set(libsamplerate_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF
    -DLIBSAMPLERATE_EXAMPLES=OFF)

ExternalProject_Add(
    libsamplerate
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libsamplerate
    GIT_REPOSITORY ${libsamplerate_GIT_REPOSITORY}
    GIT_TAG ${libsamplerate_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${libsamplerate_ARGS})
