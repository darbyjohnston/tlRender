include(ExternalProject)

set(USD_DEPS)

set(USD_ARGS)
if(CMAKE_OSX_ARCHITECTURES)
    list(APPEND USD_ARGS --build-target ${CMAKE_OSX_ARCHITECTURES})
endif()
if(CMAKE_OSX_DEPLOYMENT_TARGET)
    list(APPEND USD_ARGS --build-args)
    list(APPEND USD_ARGS USD,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS OpenSubdiv,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    list(APPEND USD_ARGS MaterialX,"-DCMAKE_OSX_DEPLOYMENT_TARGET=${CMAKE_OSX_DEPLOYMENT_TARGET}")
    #list(APPEND USD_ARGS TBB,"CFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET} CXXFLAGS=-mmacosx-version-min=${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
list(APPEND USD_ARGS --no-python)

ExternalProject_Add(
    USD
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/USD
    DEPENDS ${USD_DEPS}
    URL https://github.com/PixarAnimationStudios/USD/archive/refs/tags/v23.05.tar.gz
    CONFIGURE_COMMAND ""
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_SOURCE_DIR}/USD-patch/build_usd.py
        ${CMAKE_CURRENT_BINARY_DIR}/USD/src/USD/build_scripts/build_usd.py
    BUILD_COMMAND python3 build_scripts/build_usd.py ${USD_ARGS} ${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND "")

