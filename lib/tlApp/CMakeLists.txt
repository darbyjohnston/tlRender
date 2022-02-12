set(HEADERS
    CmdLine.h
    CmdLineInline.h
    IApp.h)
set(SOURCE
    CmdLine.cpp
    IApp.cpp)

add_library(tlApp ${HEADERS} ${SOURCE})
target_link_libraries(tlApp tlCore)
set_target_properties(tlApp PROPERTIES FOLDER lib)
