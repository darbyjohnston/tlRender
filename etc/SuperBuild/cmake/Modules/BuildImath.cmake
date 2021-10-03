include(ExternalProject)

set(Imath_GIT_REPOSITORY "https://github.com/AcademySoftwareFoundation/Imath.git")
set(Imath_GIT_TAG "6dc0820482aeb24a22f520d7b165a410589648c3") # v3.1.3

set(Imath_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DBUILD_TESTING=OFF)

ExternalProject_Add(
    Imath
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/Imath
    GIT_REPOSITORY ${Imath_GIT_REPOSITORY}
    GIT_TAG ${Imath_GIT_TAG}
    LIST_SEPARATOR |
    CMAKE_ARGS ${Imath_ARGS})
