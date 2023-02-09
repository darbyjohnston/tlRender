include( ExternalProject )

# Use this for cuting edge yasm
# GIT_REPOSITORY "https://github.com/yasm/yasm.git"

ExternalProject_Add(
  YASM
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/YASM
  URL "http://www.tortall.net/projects/yasm/releases/yasm-1.3.0.tar.gz"
  CMAKE_ARGS
  -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
  -DCMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}
  -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DYASM_BUILD_TESTS=OFF
  -DBUILD_SHARED_LIBS=FALSE
  )

