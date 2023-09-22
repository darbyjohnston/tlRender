set(HEADERS
    IApp.h)

set(SOURCE
    IApp.cpp)

set(LIBRARIES tlApp tlUI)

add_library(tlGLApp ${HEADERS} ${SOURCE})
target_link_libraries(tlGLApp ${LIBRARIES})
set_target_properties(tlGLApp PROPERTIES FOLDER lib)
set_target_properties(tlGLApp PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlGLApp
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGLApp)
