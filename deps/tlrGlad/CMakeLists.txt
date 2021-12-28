set(HEADERS gl.h khrplatform.h)
set(SOURCE gl.c)

add_library(tlrGlad ${HEADERS} ${SOURCE})
target_link_libraries(tlrGlad OpenGL::GL)
set_target_properties(tlrGlad PROPERTIES FOLDER deps)
set_target_properties(tlrGlad PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlrGlad
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlrGlad)
