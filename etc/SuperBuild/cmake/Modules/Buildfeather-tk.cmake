include(ExternalProject)

set(feather_tk_GIT_REPOSITORY "https://github.com/darbyjohnston/feather-tk.git")
set(feather_tk_GIT_TAG "7938309f4ff8e0e7daf6c9f1280e749f3a2c7733")

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
    ftk-sbuild
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk-sbuild
    DEPENDS ${feather_tk_DEPS}
    GIT_REPOSITORY ${feather_tk_GIT_REPOSITORY}
    GIT_TAG ${feather_tk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${feather_tk_ARGS})

ExternalProject_Add(
    feather-tk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk
    DEPENDS ftk-sbuild
    GIT_REPOSITORY ${feather_tk_GIT_REPOSITORY}
    GIT_TAG ${feather_tk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${feather_tk_ARGS})
