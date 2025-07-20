include(ExternalProject)

set(feather_tk_GIT_REPOSITORY "https://github.com/darbyjohnston/feather-tk.git")
set(feather_tk_GIT_TAG "808cea4fd52d03c9eb391b2763cb31878c6c5edb")

set(feather_tk_DEPS ZLIB PNG)
set(feather_tk_ARGS
    -Dfeather_tk_API=${feather_tk_API}
    -Dfeather_tk_ZLIB=OFF
    -Dfeather_tk_PNG=OFF
    -Dfeather_tk_PYTHON=OFF
    -Dfeather_tk_TESTS=OFF
    -Dfeather_tk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    feather-tk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/feather-tk
    DEPENDS ${feather_tk_DEPS}
    GIT_REPOSITORY ${feather_tk_GIT_REPOSITORY}
    GIT_TAG ${feather_tk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${feather_tk_ARGS})
