set(HEADERS
	gl.h
	khrplatform.h)

set(SOURCE
	gl.c)

add_library(tlGlad ${HEADERS} ${SOURCE})
target_link_libraries(tlGlad OpenGL::GL ${CMAKE_DL_LIBS})
set_target_properties(tlGlad PROPERTIES FOLDER deps)
set_target_properties(tlGlad PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(
    TARGETS tlGlad
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    PUBLIC_HEADER DESTINATION include/tlRender/tlGlad)
