set(HEADERS
    App.h)
set(SOURCE
    App.cpp)

add_library(tlAppBake ${HEADERS} ${SOURCE})
target_link_libraries(tlAppBake tlApp tlGL GLFW)
set_target_properties(tlAppBake PROPERTIES FOLDER lib)
set_target_properties(tlAppBake PROPERTIES PUBLIC_HEADER "${HEADERS}")

install(TARGETS tlAppBake
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin
        PUBLIC_HEADER DESTINATION include/tlRender/tlAppBake)
