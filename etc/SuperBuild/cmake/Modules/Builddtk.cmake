include(ExternalProject)

set(dtk_GIT_REPOSITORY "https://github.com/darbyjohnston/dtk.git")
set(dtk_GIT_TAG "671fd0bd0a4f3707d096960b1a51f3af79b7d844")

set(dtk_DEPS dtk-deps)
set(dtk_ARGS
    -Ddtk_API=${dtk_API}
    -Ddtk_PYTHON=OFF
    -Ddtk_TESTS=OFF
    -Ddtk_EXAMPLES=OFF
    ${TLRENDER_EXTERNAL_ARGS})

ExternalProject_Add(
    dtk
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/dtk
    DEPENDS ${dtk_DEPS}
    GIT_REPOSITORY ${dtk_GIT_REPOSITORY}
    GIT_TAG ${dtk_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${dtk_ARGS})
