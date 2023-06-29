add_executable(tlplay-gl main.cpp)
target_link_libraries(tlplay-gl tlPlayGLApp)
set_target_properties(tlplay-gl PROPERTIES FOLDER bin)

install(
    TARGETS tlplay-gl
    RUNTIME DESTINATION bin)
