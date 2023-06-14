include(ExternalProject)

set(USD_DEPS)

ExternalProject_Add(
    USD
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/USD
    DEPENDS ${USD_DEPS}
    URL https://github.com/PixarAnimationStudios/USD/archive/refs/tags/v23.05.tar.gz
    CONFIGURE_COMMAND ""
    BUILD_COMMAND python build_scripts/build_usd.py --no-python ${CMAKE_INSTALL_PREFIX}
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND "")

