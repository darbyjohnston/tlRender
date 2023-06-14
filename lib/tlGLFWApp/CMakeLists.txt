set(HEADERS
    IApp.h)

set(SOURCE
    IApp.cpp)

set(LIBRARIES tlApp tlUI tlTimeline)

add_library(tlGLFWApp ${HEADERS} ${SOURCE})
target_link_libraries(tlGLFWApp ${LIBRARIES})
set_target_properties(tlGLFWApp PROPERTIES FOLDER lib)
set_target_properties(tlGLFWApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGLFWApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGLFWApp)
