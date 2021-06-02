include(ExternalProject)

set(FreeType_ARGS
    ${TLR_EXTERNAL_ARGS}
    -DCMAKE_INSTALL_LIBDIR=lib
    -DFT_WITH_ZLIB=ON
    -DCMAKE_DISABLE_FIND_PACKAGE_BZip2=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_PNG=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_HarfBuzz=TRUE
    -DCMAKE_DISABLE_FIND_PACKAGE_BrotliDec=TRUE)
if(CMAKE_CXX_STANDARD)
    set(FreeType_ARGS ${FreeType_ARGS} -DCMAKE_CXX_STANDARD=${CMAKE_CXX_STANDARD})
endif()

ExternalProject_Add(
    FreeType
    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/FreeType
    DEPENDS ZLIB
    URL http://download.savannah.gnu.org/releases/freetype/freetype-2.10.2.tar.gz
    CMAKE_ARGS ${FreeType_ARGS})
