include(ExternalProject)

set(ftk_GIT_REPOSITORY "https://github.com/darbyjohnston/feather-tk.git")
set(ftk_GIT_TAG "6ab53505f083ad96f7fc3c23bec514f91e6a4e0c")

set(ftk_DEPS ZLIB PNG)
set(ftk_ARGS
    -Dftk_API=${ftk_API}
    -Dftk_ZLIB=OFF
    -Dftk_PNG=OFF
    -Dftk_PYTHON=OFF
    -Dftk_TESTS=OFF
    -Dftk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    ftk-sbuild
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk-sbuild
    DEPENDS ${ftk_DEPS}
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    INSTALL_COMMAND ""
    SOURCE_SUBDIR etc/SuperBuild
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})

ExternalProject_Add(
    ftk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ftk
    DEPENDS ftk-sbuild
    GIT_REPOSITORY ${ftk_GIT_REPOSITORY}
    GIT_TAG ${ftk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${ftk_ARGS})
