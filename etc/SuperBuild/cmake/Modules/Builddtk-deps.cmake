include(ExternalProject)

set(dtk_GIT_REPOSITORY "https://github.com/darbyjohnston/dtk.git")
set(dtk_GIT_TAG "a3fb5fbe8d6064b5d6cbc217b26415f86c0b30f5")

set(dtk-deps_ARGS
    -Ddtk_API=${dtk_API}
    -Ddtk_ZLIB=OFF
    -Ddtk_nlohmann_json=OFF
    -Ddtk_PNG=OFF
    -Ddtk_DEPS_ONLY=ON
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    dtk-deps
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dtk-deps
    DEPENDS ZLIB PNG
    GIT_REPOSITORY ${dtk_GIT_REPOSITORY}
    GIT_TAG ${dtk_GIT_TAG}
    SOURCE_SUBDIR etc/SuperBuild
    INSTALL_COMMAND ""
    LIST_SEPARATOR |
    CMAKE_ARGS ${dtk-deps_ARGS})
