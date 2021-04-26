set(HEADERS
    CmdLine.h
    CmdLineInline.h
    IApp.h
    Util.h)
set(SOURCE
    CmdLine.cpp
    IApp.cpp
    Util.cpp)

add_library(tlrApp ${HEADERS} ${SOURCE})
target_link_libraries(tlrApp tlrRender)
set_target_properties(tlrApp PROPERTIES FOLDER lib)
