include(ExternalProject)

set(dtk_GIT_REPOSITORY "https://github.com/darbyjohnston/dtk.git")
set(dtk_GIT_TAG "a18ba0cfedd430bda999a9957ce94e1dee3bd039")

set(dtk-deps_ARGS
    -Ddtk_ZLIB=OFF
    -Ddtk_nlohmann_json=OFF
    -Ddtk_PNG=OFF
    -Ddtk_Freetype=OFF
    -Ddtk_glfw3=OFF
    -Ddtk_NFD=OFF
    -Ddtk_DEPS_ONLY=ON
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    dtk-deps
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dtk-deps
    DEPENDS ZLIB PNG Freetype
    GIT_REPOSITORY ${dtk_GIT_REPOSITORY}
    GIT_TAG ${dtk_GIT_TAG}
    SOURCE_SUBDIR etc/SuperBuild
    INSTALL_COMMAND ""
    LIST_SEPARATOR |
    CMAKE_ARGS ${dtk-deps_ARGS})
