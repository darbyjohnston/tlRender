set(HEADERS
    gl.h
    khrplatform.h)

set(SOURCE
    gl.c)

add_library(tlGladDebug ${HEADERS} ${SOURCE})
target_link_libraries(tlGladDebug OpenGL::GL ${CMAKE_DL_LIBS})
set_target_properties(tlGladDebug PROPERTIES FOLDER deps)
set_target_properties(tlGladDebug PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlGladDebug
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGladDebug)
