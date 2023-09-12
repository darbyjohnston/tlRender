add_executable(tlplay-gl WIN32 main.cpp)
target_link_libraries(tlplay-gl tlPlayGLApp)
set_target_properties(tlplay-gl PROPERTIES FOLDER bin)

install(
    TARGETS tlplay-gl
    RUNTIME DESTINATION bin)

if(WIN32)
    add_executable(tlplay-gl. main.cpp)
    target_link_libraries(tlplay-gl. tlPlayGLApp)    
    set_target_properties(
        tlplay-gl. PROPERTIES
        SUFFIX com
        FOLDER bin)
    install(
        TARGETS tlplay-gl.
        RUNTIME DESTINATION bin)
endif()
