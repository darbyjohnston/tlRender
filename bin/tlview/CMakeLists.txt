add_executable(tlview main.cpp)
target_link_libraries(tlview tlViewApp)
set_target_properties(tlview PROPERTIES FOLDER bin)

install(
    TARGETS tlview
    RUNTIME DESTINATION bin)
