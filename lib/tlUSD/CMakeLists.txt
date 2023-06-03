set(HEADERS
    USD.h
    USDPrivate.h
    Util.h)
set(PRIVATE_HEADERS)

set(SOURCE
    USD.cpp
    USDRead.cpp
    USDRenderer.cpp
    Util.cpp)

add_library(tlUSD ${HEADERS} ${PRIVATE_HEADERS} ${SOURCE})
target_link_libraries(tlUSD tlIO tlGL GLFW tf usd usdGeom usdImagingGL usdAppUtils hd)
set_target_properties(tlUSD PROPERTIES FOLDER lib)
set_target_properties(tlUSD PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlUSD
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlUSD)
