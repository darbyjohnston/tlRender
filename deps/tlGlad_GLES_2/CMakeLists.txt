set(HEADERS
    include/EGL/eglplatform.h
    include/glad/egl.h
    include/glad/gles2.h
    include/KHR/khrplatform.h)

set(SOURCE
    src/egl.c
    src/gles2.c)

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
