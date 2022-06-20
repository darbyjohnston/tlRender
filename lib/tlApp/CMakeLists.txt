set(HEADERS
    CmdLine.h
    CmdLineInline.h
    IApp.h)

set(SOURCE
    CmdLine.cpp
    IApp.cpp)

add_library(tlApp ${HEADERS} ${SOURCE})
target_link_libraries(tlApp tlTimeline)
set_target_properties(tlApp PROPERTIES FOLDER lib)
set_target_properties(tlApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlApp)
