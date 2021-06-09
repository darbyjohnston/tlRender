set(HEADERS
    CmdLineTest.h)
set(SOURCE
    CmdLineTest.cpp)

add_library(tlrAppTest ${SOURCE} ${HEADERS})
target_link_libraries(tlrAppTest tlrTestLib tlrApp)
set_target_properties(tlrAppTest PROPERTIES FOLDER tests)
