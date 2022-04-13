set(HEADERS
    CmdLineTest.h)

set(SOURCE
    CmdLineTest.cpp)

add_library(tlAppTest ${SOURCE} ${HEADERS})
target_link_libraries(tlAppTest tlTestLib tlApp)
set_target_properties(tlAppTest PROPERTIES FOLDER tests)
