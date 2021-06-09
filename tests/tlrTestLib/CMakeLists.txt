set(HEADERS
    ITest.h)
set(SOURCE
    ITest.cpp)

add_library(tlrTestLib ${SOURCE} ${HEADERS})
target_link_libraries(tlrTestLib tlrCore)
set_target_properties(tlrTestLib PROPERTIES FOLDER tests)
