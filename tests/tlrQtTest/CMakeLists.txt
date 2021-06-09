set(HEADERS
    TimeObjectTest.h)
set(SOURCE
    TimeObjectTest.cpp)

add_library(tlrQtTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrQtTest tlrTestLib tlrQt)
set_target_properties(tlrQtTest PROPERTIES FOLDER tests)
