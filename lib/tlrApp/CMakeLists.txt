set(HEADERS
    CmdLine.h
    CmdLineInline.h
    IApp.h)
set(SOURCE
    CmdLine.cpp
    IApp.cpp)

add_library(tlrApp ${HEADERS} ${SOURCE})
target_link_libraries(tlrApp tlrCore)
set_target_properties(tlrApp PROPERTIES FOLDER lib)
